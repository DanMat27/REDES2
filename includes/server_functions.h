/* 
* File: server_functions.h
* Authors: DanMat27
*		   AMP
*/
#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include "../includes/server.h"

/********
* FUNCION: CONTROL leer_fichero_conf(FILE* fichero, config* conf);
* ARGS_IN: config* conf - estructura donde guardamos caracteristicas del servidor
*          
* DESCRIPCION: Lee el fichero de configuracion del servidor para conocer las
*              propiedades del mismo y almacena la info en la estructura de 
*              configuracion.
* ARGS_OUT: CONTROL - OK, en caso de que todo vaya bien. ERROR, en caso contrario
********/
CONTROL leer_fichero_conf(config* conf);


/********
* FUNCION: config* reservar_conf();
* ARGS_IN: void
*          
* DESCRIPCION: Reserva memoria para la estructura de configuracion.
*              
* ARGS_OUT: config - La estructura si se reservó bien, NULL si no.
********/
config* reservar_conf();


/********
* FUNCION: void liberar_conf(config* conf);
* ARGS_IN: config* conf - estructura donde guardamos caracteristicas del servidor 
*          
* DESCRIPCION: Libera la memoria de la estructura de configuracion.
*              
* ARGS_OUT: void
********/
void liberar_conf(config* conf);


/********
* FUNCION: void manejador_Control_C(int sig);
* ARGS_IN: int sig - entero con la senial capturada
*          
* DESCRIPCION: Se activa al realizar un ctrl+C y pone el servidor inactivo
*              
* ARGS_OUT: void
********/
void manejador_Control_C(int sig);


/********
* FUNCION: void procesar_peticion(char* msg, config* conf, ssize_t buflen, char* cabecera)
* ARGS_IN: char* msg - buffer con la peticion del cliente 
*          config* conf - estructura donde guardamos caracteristicas del servidor   
*          ssize_t buflen - longitud de la peticion leida
*          char* cabecera - buffer con la cabecera a enviar 
*          int clientfd - descriptor del cliente
*
* DESCRIPCION: Procesa la peticion de un cliente y devuelve el mensaje de cabecera
*
* ARGS_OUT: void
********/
void procesar_peticion(char* msg, config* conf, ssize_t buflen, char* cabecera, int clientfd);


/********
* FUNCION: void procesa_get_post(char* path, char* cabecera, config* conf)
* ARGS_IN: char* path - path del recurso solicitado
*          char* cabecera - buffer con la cabecera a enviar  
*          config* conf - estructura donde guardamos caracteristicas del servidor
*
* DESCRIPCION: Procesa una peticion GET o POST, buscando en el path el recurso y construye
*              la cabecera
*
* ARGS_OUT: int - 1 si es script, 0 si no y n_error si error.
********/
int procesa_get_post(char* path, char* cabecera, config* conf);


/********
* FUNCION: void procesa_options(char* cabecera, config* conf)
* ARGS_IN: char* cabecera - buffer con la cabecera a enviar  
*          config* conf - estructura donde guardamos caracteristicas del servidor
*
* DESCRIPCION: Procesa una peticion OPTIONS, mostrando las peticiones
*              soportadas y construye la cabecera
*
* ARGS_OUT: void
********/
void procesa_options(char* cabecera, config* conf);


/********
* FUNCION: char* crea_buffer(int tam);
* ARGS_IN: int tam - tamanio del nuevo buffer
*          
* DESCRIPCION: Reserva memoria para un buffer de chars
*
* ARGS_OUT: char* - buffer reservado
********/
char* crea_buffer(int tam);


/********
* FUNCION: void libera_buffer(char* buffer)
* ARGS_IN: char* buffer - buffer para liberar
*
* DESCRIPCION: Libera la memoria reservada a un buffer
*
* ARGS_OUT: void
********/
void libera_buffer(char* buffer);


/********
* FUNCION: CONTROL envia_por_trozos(int clientfd, char *buffer, int len);
* ARGS_IN: int clientfd - descriptor del cliente
*          char* buffer - buffer del mensaje a enviar
*          char* path - path del recurso solicitado
*          char* path_args - buffer con los argumentos del get (null si no hay)
*          char* body_args - buffer con los argumentos del post 
*          config* conf - estructura donde guardamos caracteristicas del servidor
*          int tipo - tipo de peticion (GET, POST)
*          int script - entero que indica si el mensaje es un script (1)
*          
* DESCRIPCION: Envia por trozos un mensaje al cliente leyendo el recurso tambien
*              en trozos iguales
*
* ARGS_OUT: void
********/
void envia_por_trozos(int clientfd, char* buffer, char* path, char* path_args, char* body_args, config* conf, int tipo, int script);

/********
* FUNCION: void obtiene_args_path(char* command, char* path_args);
* ARGS_IN: char* command - buffer con el comando
*          char* path_args - buffer con los argumentos del path
*          
* DESCRIPCION: Obtiene los argumentos del path y los introduce en
*              el comando
*
* ARGS_OUT: void
********/
void obtiene_args_path(char* command, char* path_args);

/********
* FUNCION: void send_error_html(int clientfd, int error, char* cabecera, config* conf);
* ARGS_IN: int clientfd - descriptor del cliente
*          int error - codigo del error
*          char* cabecera - cabecera de la respuesta
*          config* conf - estructura donde guardamos caracteristicas del servidor
*          
* DESCRIPCION: Envia un html con el codigo de error correspondiente
*
* ARGS_OUT: void
********/
void send_error_html(int clientfd, int error, char* cabecera, config* conf);

#endif /* SERVER_FUNCTIONS_H */