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

    void* payload = obtener_payload(fd_worker);
    //int bytes_recibidos = recv(fd_worker, &payload, sizeof(int), MSG_WAITALL);

    int offset = 0;
    int id_worker = obtener_un_entero(payload, &offset);

    log_info(logger, "## Se conecta el Worker %d - Cantidad total de Workers: %d", id_worker, cantidad_actual); // Log obligatorio(mirar enunciado)

    while (1){
        t_header header = obtener_header(fd_worker);
        if (header == -1) {
            break;
        }
        
    }

    // Worker se desconectó - decrementar contador
    pthread_mutex_lock(&mutex_workers);
    workers_conectados--;
    cantidad_actual = workers_conectados;
    pthread_mutex_unlock(&mutex_workers);
    
    // Log obligatorio: Desconexión de Worker
    log_info(logger, "## Se desconecta el Worker %d - Cantidad de Workers: %d", id_worker, cantidad_actual);
    
    close(fd_worker);
    free(fd_worker_ptr);
    destruir_payload(payload);

    return NULL;
}