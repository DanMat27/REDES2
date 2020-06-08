/* 
* File: threads.h
* Authors: DanMat27
*		   AMP
*/

#ifndef THREADS_H
#define THREADS_H

/********
* FUNCION: void* resolver_peticion(void* cliente)
* ARGS_IN: void* hilo - estructura con los datos de la peticion para cliente
* DESCRIPCIÓN: resuelve una peticion con el hilo del cliente
* ARGS_OUT: void - nada
********/
void resolver_peticion(void* hilo);

/********
* FUNCION: pthread_t* crear_hilo(int* client)
* ARGS_IN: Hilo* hilo - estructura con los datos de la peticion para cliente
* DESCRIPCIÓN: Crea un hilo por cada cliente
* ARGS_OUT: void - nada
********/
void crear_hilo(Hilo* hilo);


#endif /* THREADS_H */