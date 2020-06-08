/* 
* File: server.c
* Authors: DanMat27
*		   AMP
*/

#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "../includes/server.h"
#include "../includes/threads.h"
#include "../includes/server_functions.h"

int main(void)
{   int sockfd;
	struct sockaddr_in server;
	Hilo* hilo=NULL;
	struct sigaction act;

	// Reservamos memoria para la estructura con los parametros del hilo
	hilo = (Hilo*)calloc(1, sizeof(Hilo));
	if(!hilo) {
		return ERROR;
	}

	// Reservamos memoria para la estructura de configuracion
	hilo->conf = reservar_conf();
    if(!hilo->conf){
		free(hilo);
        return ERROR;
    }

	// Leemos el fichero de configuracion
	if(leer_fichero_conf(hilo->conf) == ERROR){
		liberar_conf(hilo->conf);
		free(hilo);
		perror("Error al leer fichero de configuracion");
		exit(errno);
	}

	//Reservamos buffers para mensajes
	hilo->buffer = crea_buffer(MAXBUF);
	if(!hilo->buffer){
		liberar_conf(hilo->conf);
		free(hilo);
		perror("Error al reservar buffer");
		exit(errno);
	}

	hilo->cabecera = crea_buffer(MAXBUF);
	if(!hilo->cabecera){
		liberar_conf(hilo->conf);
		libera_buffer(hilo->buffer);
		free(hilo);
		perror("Error al reservar buffer");
		exit(errno);
	}

	// Creamos el socket tipo TCP */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		liberar_conf(hilo->conf);
		libera_buffer(hilo->buffer);
		libera_buffer(hilo->cabecera);
		free(hilo);
		perror("Error en la creación del socket");
		exit(errno);
	}

	// Inicializamos estructura de dirección y puerto
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(hilo->conf->num_puerto);
	server.sin_addr.s_addr = inet_addr(hilo->conf->ip_server);

	// Ligamos puerto al socket
    if (bind(sockfd, (struct sockaddr*)&server, sizeof(server)) != 0){
		close(sockfd);
		liberar_conf(hilo->conf);
		libera_buffer(hilo->buffer);
		libera_buffer(hilo->cabecera);
		free(hilo);
		perror("socket--bind");
		exit(errno);
	}

	// Establecemos cola de peticiones
	if ( listen(sockfd, 20) != 0 ){
		close(sockfd);
		liberar_conf(hilo->conf);
		libera_buffer(hilo->buffer);
		libera_buffer(hilo->cabecera);
		free(hilo);
		perror("socket--listen");
		exit(errno);
	}

	/* Declaramos mascara de seniales */
	sigemptyset(&(act.sa_mask));
	act.sa_flags = 0;

	/* Capturamos la senial SIGINT cuando se realiza un Control+C */
	act.sa_handler = manejador_Control_C; 
	if(sigaction(SIGINT, &act, NULL)<0){
		close(sockfd);
		liberar_conf(hilo->conf);
		libera_buffer(hilo->buffer);
		libera_buffer(hilo->cabecera);
		free(hilo);
		perror("Error en la captura de la senial SIGINT");
		exit(errno);
	}

	/* Servidor activo. Empieza a escuchar peticiones */
    servidor_activo = 1;

	fprintf(stdout, "\n----Instrucciones----\n");
	fprintf(stdout, "Para detener el servidor se ha de realizar Ctrl+C!\n\n");
	fprintf(stdout, "----Servidor Activo----\nEscuchando en [%s:%d]...\n\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

	// Nota: habra que crear una lista de hilos de para cada cliente???
	while (servidor_activo){	
		struct sockaddr_in client_addr;
		int addrlen=sizeof(client_addr);

		// Aceptamos conexiones de clientes
		hilo->cliente = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
    	fprintf(stdout, "\n#########################\n");
		printf("----Peticion Recibida----\nConexión desde [%s:%d]\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		//Recibimos una peticion
		hilo->tam = recv(hilo->cliente, hilo->buffer, MAXBUF, 0);

		// Se procesa la peticion con un hilo por cliente
		crear_hilo(hilo);
	}

	fprintf(stdout, "\n#########################\n");
    fprintf(stdout, "----Servidor detenido----\n");

	// Liberamos recursos del servidor
	close(sockfd);
	liberar_conf(hilo->conf);
	libera_buffer(hilo->buffer);
	libera_buffer(hilo->cabecera);
	free(hilo);

	fprintf(stdout, "Recursos liberados correctamente.\n");
	fprintf(stdout, "#########################\n\n");

	exit(EXIT_SUCCESS);
}