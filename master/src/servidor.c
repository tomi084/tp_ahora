#include "servidor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

#include <commons/log.h>

#include <utils/conexiones.h>
#include <utils/serializacion.h>
#include <utils/deserializacion.h>

#include "config.h"
#include "hilo-worker.h"
#include "hilo-query_control.h"
#include "serializacion.h"


extern t_log* logger;
extern t_config_master* config_master;

static int fd_servidor_master = -1;
static pthread_t thread;

volatile bool programa_activo = false;

static int _iniciar_servidor_master();
static void* _esperar_clientes(void* arg);
static void* _handshake(void* arg);

void inicializar_servidor(){
    fd_servidor_master = _iniciar_servidor_master();
    int* fd_servidor_master_ptr = malloc(sizeof(int));
    *fd_servidor_master_ptr = fd_servidor_master;

    pthread_create(&thread, NULL, _esperar_clientes, fd_servidor_master_ptr);

}

static int _iniciar_servidor_master(){
    char* puerto_escucha = obtener_puerto_escucha();
    int fd_servidor = crear_servidor(puerto_escucha);

    if(fd_servidor == -1) {
        log_error(logger, "No se ha podido inicializar el servidor Master en el puerto %s.", puerto_escucha);
        exit(EXIT_FAILURE);
        return -1;
    }

    log_info(logger, "Servidor Master Inicializado - Puerto: %s", puerto_escucha);

    free(puerto_escucha);

    programa_activo = true;

    return fd_servidor;
}

static void* _esperar_clientes(void* arg){
    int fd_master = *((int*) arg);
    free(arg);

    while (programa_activo) {
        int fd_cliente = aceptar_cliente(fd_master);

        if (fd_cliente == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // No hay conexiones pendientes, esperar un poco
                usleep(100000); // 100ms
                continue;
            }
            
            if (programa_activo) {
                log_error(logger, "Error en accept: %s", strerror(errno));
            }
            break;
        }

        int* fd_cliente_ptr = malloc(sizeof(int));
        *fd_cliente_ptr = fd_cliente;

        pthread_t thread_cliente;
        pthread_create(&thread_cliente, NULL, _handshake, fd_cliente_ptr);
        pthread_detach(thread_cliente);
    }

    log_info(logger, "Cerrando servidor Master...");
    finalizar_servidor(fd_master);
    finalizar_programa();
    
    return NULL;
}

static void* _handshake(void* arg){ 
    int* fd_master_ptr = (int*)arg;

    t_header header = obtener_header(*fd_master_ptr);
    //void* payload = obtener_payload(*fd_master_ptr);
    
    switch (header) {
        case HANDSHAKE_WORKER: {
            t_paquete* respuesta = serializar_respuesta_handshake_worker();
            enviar_paquete(respuesta, *fd_master_ptr);

            //int offset = 0;
            //int id_worker = obtener_un_entero(payload, &offset);

            //log_info(logger, "ID: %d", id_worker); // es prueba
            
            pthread_t worker;
            pthread_create(&worker, NULL, atender_worker, fd_master_ptr);
            pthread_detach(worker);
            
            break;
        }
        case HANDSHAKE_QUERY_CONTROL:{
            t_paquete* respuesta = serializar_respuesta_handshake_query_control();
            enviar_paquete(respuesta, *fd_master_ptr);

            
            
            pthread_t worker;
            pthread_create(&worker, NULL, atender_query_control, fd_master_ptr);
            pthread_detach(worker);
            
            break;
        }
        default: {
            t_paquete* respuesta = serializar_respuesta_handshake_desconocido();
            enviar_paquete(respuesta, *fd_master_ptr);
            
            log_error(logger, "Valor no identificado en el Handshake");
            
            close(*fd_master_ptr);
            free(fd_master_ptr);
 
            break;
        }
    }

    //destruir_payload(payload);

    return NULL;
}

void finalizar_programa() {
    if (config_master) {
        liberar_config_master(config_master);
    }
    if (logger) {
        log_info(logger, "Master finalizado correctamente");
        log_destroy(logger);
    }
}