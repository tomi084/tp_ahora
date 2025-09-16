#include <string.h>

#include <utils/estructuras.h>
#include <utils/serializacion.h>

#include "serializacion.h"

t_paquete* serializar_handshake_master(char* archivo_query, int prioridad){
	t_paquete* paquete = crear_paquete(HANDSHAKE_QUERY_CONTROL);
	agregar_a_paquete(paquete, &archivo_query, sizeof(archivo_query));
    agregar_a_paquete(paquete, &prioridad, sizeof(prioridad));
	return paquete;
}
