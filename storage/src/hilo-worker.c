#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>

#include <commons/log.h>
#include <commons/string.h>

#include <utils/estructuras.h>
#include <utils/serializacion.h>
#include <utils/deserializacion.h>

#include "hilo-worker.h"
#include "serializacion.h"
#include "deserializacion.h"
#include "config.h"

extern t_log* logger;

// Contador global de workers conectados (thread-safe)
static int workers_conectados = 0;
static pthread_mutex_t mutex_workers = PTHREAD_MUTEX_INITIALIZER;

void* atender_worker(void* arg) {

    int* fd_worker_ptr = (int*)arg;
    int fd_worker = *fd_worker_ptr;
    
    // Incrementar contador de workers conectados
    pthread_mutex_lock(&mutex_workers);
    workers_conectados++;
    int cantidad_actual = workers_conectados;
    pthread_mutex_unlock(&mutex_workers);

    log_info(logger, "## Se conecta el Worker %d - Cantidad de Workers: %d", fd_worker, cantidad_actual);

    t_header header;
    void* payload;
    int bytes_recibidos = recv(fd_worker, &header, sizeof(int), MSG_WAITALL);
    
    while (bytes_recibidos > 0){

        usleep(100); // simulando ejecucion
        /*
        payload = obtener_payload(fd_worker);
        t_deserializado* operacion = deserializar(header, payload);

        // TODO: Procesar diferentes tipos de peticiones
        switch (header) {
            case CREATE_FILE:
                log_info(logger, "Worker %d solicita CREATE_FILE", fd_worker);
                // TODO: Implementar CREATE_FILE
                // ejemplo:
                // procesar_create_file(operacion);
                break;
                
            // TODO: Implementar todas las operaciones  
            
            default:
                log_warning(logger, "Worker %d envió header desconocido: %d", fd_worker, header);
                break;
           
           } */
        
        destruir_payload(payload);
        //destruir_deserializado(operacion);
        bytes_recibidos = recv(fd_worker, &header, sizeof(int), MSG_WAITALL);
        
    }

    // Worker se desconectó - decrementar contador
    pthread_mutex_lock(&mutex_workers);
    workers_conectados--;
    cantidad_actual = workers_conectados;
    pthread_mutex_unlock(&mutex_workers);
    
    // Log obligatorio: Desconexión de Worker
    log_info(logger, "## Se desconecta el Worker %d - Cantidad de Workers: %d", fd_worker, cantidad_actual);
    
    close(fd_worker);
    free(fd_worker_ptr);

    return NULL;
}