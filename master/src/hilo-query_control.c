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
//#include "deserializacion.h"
#include "config.h"

extern t_log* logger;

// Contador global de queries conectados (thread-safe)
static int queries_conectados = 0;
static pthread_mutex_t mutex_queries = PTHREAD_MUTEX_INITIALIZER;

void* atender_query_control(void* arg) {

    int* fd_query_ptr = (int*)arg;
    int fd_query = *fd_query_ptr;

    // Incrementar contador de workers conectados
    pthread_mutex_lock(&mutex_queries);
    queries_conectados++;
    int cantidad_actual = queries_conectados;
    pthread_mutex_unlock(&mutex_queries);

    void* payload = obtener_payload(fd_query);
    int offset = 0;
    char* path_query = obtener_un_string(payload, &offset);
    int prioridad = obtener_un_entero(payload, &offset);

    log_info(logger, "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: . Nivel multiprogramación %d", path_query, prioridad, cantidad_actual); // log obligatorio (mirar enunciado)


    while (1){
        t_header header = obtener_header(fd_query);

        if (header == -1) {
            break;
        }
        
    }

    // Worker se desconectó - decrementar contador
    pthread_mutex_lock(&mutex_queries);
    queries_conectados--;
    cantidad_actual = queries_conectados;
    pthread_mutex_unlock(&mutex_queries);

    // Log obligatorio: Desconexión de Worker
    log_info(logger, "## Se desconecta un Query Control. Se finaliza la Query <QUERY_ID> con prioridad %d. Nivel de multiprogramación %d", prioridad, cantidad_actual);

    close(fd_query);
    free(fd_query_ptr);

    return NULL;
}