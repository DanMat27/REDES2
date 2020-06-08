/* 
* File: server.h
* Authors: DanMat27
*		   AMP
*/
#ifndef SERVER_H
#define SERVER_H

#define MAXBUF 2048
#define MOREBUF 4096
#define MSG 240
#define IPBUF 17
#define PY 1
#define PHP 2
#define GET 1
#define POST 2

/* Codigos de respuesta */
#define ERROR_400 "400 Bad Request"
#define ERROR_404 "404 Not Found"
#define ERROR_500 "500 Internal Server Error"
#define ERROR_505 "505 HTTP Version Not Supported"
#define OK_200 "200 OK"
#define RE_301 "301 Moved Permanently"

typedef struct _config{
    int num_puerto;
    char ip_server[MAXBUF];
    int num_clientes;
    int http_version;
    char signature[MAXBUF];
    char server_route[MAXBUF]; 
    long content_length;
} config;

typedef struct {
    int cliente;
    char* buffer;
    config* conf;
    ssize_t tam;
    char* cabecera;
} Hilo;

typedef enum {
    ERROR=-1,
    OK=0
} CONTROL;

int servidor_activo; /* Si 1, esta activo. Si 0, para. */

#endif /* SERVER_H */