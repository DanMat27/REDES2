/* 
* File: threads.c
* Authors: DanMat27
*		   AMP
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "../includes/server.h"
#include "../includes/threads.h"
#include "../includes/server_functions.h"

/*FUNCION: void resolver_petcion(void* cliente)*/ 
void resolver_peticion(void* hilo) {
    Hilo* hiloAux = (Hilo*)hilo;
    char* buffer = hiloAux->buffer;
    config* conf = hiloAux->conf;
    ssize_t tam = hiloAux->tam;
    char* cabecera = hiloAux->cabecera;
    int cliente = hiloAux->cliente;
    procesar_peticion(buffer, conf, tam, cabecera, cliente);
    // Cerramos la conexión
	close(cliente);
}

/*FUNCION: pthread_t* crear_hilo()*/ 
void crear_hilo(Hilo* hilo) {
    pthread_t cliente;

    //creamos el hilo del cliente
    pthread_create(&cliente, NULL, (void*) resolver_peticion, (void*)hilo);
	pthread_join(cliente, NULL);
}