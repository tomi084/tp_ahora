#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "commons/log.h"

#include "hilo-master.h"
#include "serializacion.h"
//#include "deserializacion.h"
#include "config.h"
#include "query_control.h"

extern t_log* logger;

void inicializar_query_control(char* archivo_query, int prioridad){
    inicializar_master(archivo_query, prioridad);
}