/* 
* File: server_functions.c
* Authors: DanMat27
*		   AMP
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../includes/server.h"
#include "../includes/server_functions.h"
#include "../includes/picohttpparser.h"


CONTROL leer_fichero_conf(config* conf){
    FILE *f=NULL;
    char line[MAXBUF]="";
    char* toks=NULL;
    char ip[IPBUF]="";

    f = fopen("./data/server.conf", "r");
    if(!f){
        liberar_conf(conf);
        return ERROR;
    }

    fprintf(stdout, "----Fichero de configuracion leido con exito----\n");

    /* Leemos el fichero de configuracion */
    while(fgets(line, MAXBUF, f)){

        /* Ignoramos comentarios */
        if(strncmp("//", line, 2) != 0){ 
            /* Leemos puerto servidor */
            if(strncmp("PUERTO", line, 6) == 0){ 
                toks = strtok(line + 7, "\n");
                conf->num_puerto = atoi(toks);
            }
            /* Leemos IP servidor */
            else if(strncmp("IP", line, 2) == 0){ 
                toks = strtok(line + 3, "\n");
                strcpy(ip, toks);
                strcpy(conf->ip_server, toks);
            }
            /* Leemos numero maximo de clientes concurrentes */
            else if(strncmp("CLIENTES", line, 8) == 0){ 
                toks = strtok(line + 9, "\n");
                conf->num_clientes = atoi(toks);
            }
            /* Leemos ruta por defecto del servidor */
            else if(strncmp("SERVER_ROUTE", line, 12) == 0){
                toks = strtok(line + 13, "\n");
                strcpy(conf->server_route, toks);
            }
            /* Leemos firma del servidor en mensajes */
            else if(strncmp("SIGNATURE", line, 9) == 0){
                toks = strtok(line + 10, "\n");
                strcpy(conf->signature, toks);
            }
            /* Leemos version http del servidor */
            else if(strncmp("HTTP_VERSION", line, 12) == 0){
                toks = strtok(line + 13, "\n");
                conf->http_version = atoi(toks);
            }
            /* Atributo erroneo */
            else{
                fprintf(stderr, "Atributo de configuracion invalido: %s\n", line);
            }

        }

    }

    fprintf(stdout, "Puerto: %d\nIP: %s\nClientes: %d\nRuta: %s\nFirma: %s\nVersion HTTP: %d\n", conf->num_puerto, ip, conf->num_clientes, conf->server_route, conf->signature, conf->http_version);

    fclose(f);

    return OK;
}

config* reservar_conf(){
    config* conf=NULL;

    conf = (config*)calloc(1, sizeof(config));
    if(!conf){
        return NULL;
    }

    return conf;
}

void liberar_conf(config* conf){
    if(conf){
        free(conf);
    }
}

void manejador_Control_C(int sig){
	fprintf(stdout, "\nCrl+C hecho. Liberando recursos del servidor...\n");
	servidor_activo = 0;
}


void procesar_peticion(char* msg, config* conf, ssize_t buflen, char* cabecera, int clientfd) {
    char* metodo=NULL;
    char* path=NULL;
    char* host=NULL;
    char* contenido=NULL;
    char* fin_msg=NULL;
    char* body=NULL;
    char* path_args=NULL;
    int version_http;
    int codigo;
    int tipo_peticion;
    int i, script=0, args=0;

    // Creamos buffers
    metodo = crea_buffer(MAXBUF);
    path = crea_buffer(MAXBUF);
    host = crea_buffer(MAXBUF);

    // Obtenemos el metodo de la peticion
    for(i = 0; i < (int)buflen; i++){
        if(msg[i] == ' '){
            break;
        }
        sprintf(metodo, "%s%c", metodo, msg[i]);
    }
    metodo[i] = '\0';
    
    // Obtenemos el path de la peticion 
    for(i = i + 1; i < (int)buflen; i++){
        if(msg[i] == ' '){
            break;
        }
        else if(msg[i] == '?'){
            args = 1;
            break;
        }
        else{
            sprintf(path, "%s%c", path, msg[i]);
        }
    }
    path[i] = '\0';

    // Obtenemos argumentos del path si los hay
    if(args == 1){
        path_args = crea_buffer(MAXBUF);
        for(i = i + 1; i < (int)buflen; i++){
            if(msg[i] == ' '){
                break;
            }
            sprintf(path_args, "%s%c", path_args, msg[i]);
        }
        path_args[i] = '\0';
    }

    // Obtenemos la version http de la peticion 
    for(i = i + 1; i < (int)buflen; i++){
        if(msg[i] == '\r'){
            version_http = (int)msg[i - 1] - 48;
            break;
        }
    }

    // Comprobamos si soportamos la version http
    if(version_http != conf->http_version){
        sprintf(cabecera, "HTTP/1.%d %s\r\n\r\n", version_http, ERROR_505);
        fprintf(stdout, "Error Peticion: %s\n", ERROR_505);
        write(clientfd, cabecera, strlen(cabecera));
        libera_buffer(metodo);
        libera_buffer(path);
        libera_buffer(host);
        return;
    }

    // Miramos que tipo de peticion es (GET, POST u OPTIONS)
    if(strcmp(metodo, "GET") == 0){
        tipo_peticion = GET;
        script = procesa_get_post(path, cabecera, conf);
    }
    else if(strcmp(metodo, "POST") == 0){
        tipo_peticion = POST;        
        body = crea_buffer(buflen);
        // Leemos hasta llegar al cuerpo del mensaje
        for(i=i+1; i < (int)buflen; i++) {
            if(msg[i]=='\r') {
                if(msg[i+1]=='\n' && msg[i+2]=='\r' && msg[i+3]=='\n') {
                    break;
                }
            }
        }
        // Leemos el cuerpo del mensaje
        for(i=i+4; i < (int)buflen; i++) {
            sprintf(body, "%s%c", body, msg[i]);
        }
        body[i] = '\n';
        script = procesa_get_post(path, cabecera, conf);
    }
    else if(strcmp(metodo, "OPTIONS") == 0){
        procesa_options(cabecera, conf);

        // Al ser options enviamos directamente la cabecera
        send(clientfd, cabecera, strlen(cabecera), 0);
        
        // Liberamos buffers
        libera_buffer(metodo);
        libera_buffer(path);
        libera_buffer(host);
        return;
    }
    else{
        fprintf(stdout, "Error Peticion: %s\n", ERROR_400);
        script = 400;
    }

    // En caso de que haya habido un error
    if(script==400){
        send_error_html(clientfd, 400, cabecera, conf);
        libera_buffer(metodo);
        libera_buffer(path);
        libera_buffer(host);
        return;
    }
    else if(script==404){
        send_error_html(clientfd, 404, cabecera, conf);
        libera_buffer(metodo);
        libera_buffer(path);
        libera_buffer(host);
        return;
    }
    else if(script==500){
        send_error_html(clientfd, 500, cabecera, conf);
        libera_buffer(metodo);
        libera_buffer(path);
        libera_buffer(host);
        return;
    }


    // Imprimimos valores peticion recibida
    fprintf(stdout, "\n----Cabecera Peticion----\n");
    fprintf(stdout, "Peticion de tipo: %s\n", metodo);
    fprintf(stdout, "Path: %s\n", path);
    fprintf(stdout, "Version HTTP: 1.%d\n\n", version_http);

    // Imprimimos valores cabecera y contenido (cuerpo)
    fprintf(stdout, "----Cabecera Respuesta----\n%s\n\n", cabecera);

    // Enviamos la cabecera
    send(clientfd, cabecera, strlen(cabecera)*sizeof(char), 0);

    // Reservamos buffer del contenido del mensaje
    contenido = crea_buffer(MOREBUF);

    // Enviamos el contenido
    envia_por_trozos(clientfd, contenido, path, path_args, body, conf, tipo_peticion, script);
    fin_msg = crea_buffer(MSG);
    strcpy(fin_msg, "\r\n\r\n");
    send(clientfd, fin_msg, strlen(fin_msg), 0);
    fprintf(stdout, "----Respuesta Enviada----\n");
    fprintf(stdout, "#########################\n\n\n");

    // Liberamos buffers
    if(path_args) libera_buffer(path_args);
    if(body) libera_buffer(body);  
    libera_buffer(fin_msg);
    libera_buffer(metodo);
    libera_buffer(path);
    libera_buffer(host);
    libera_buffer(contenido);
}

int procesa_get_post(char* path, char* cabecera, config* conf){
    FILE* f=NULL;
    char* full_path=NULL;
    char* content_type=NULL;
    char* date=NULL;
    char* last_modified=NULL;
    int i, j, script=0;
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    struct tm tm2;
    struct stat recurso;

    // Creamos buffers
    full_path = crea_buffer(MAXBUF);
    content_type = crea_buffer(MAXBUF);
    date = crea_buffer(MAXBUF);
    last_modified = crea_buffer(MAXBUF);

    // Inicializamos path
    strcpy(full_path, "./");

    // Buscamos y abrimos el recurso solicitado
    sprintf(full_path, "%s%s%s", full_path, conf->server_route, path);
    
    f = fopen(full_path, "rb");
    if(!f){
        fprintf(stdout, "Error Peticion: %s\n", ERROR_404);
        return 404;
    }

    //Identificamos content-type del recurso solicitado
    for(i = strlen(path) - 1; i > -1; i--){
        if(path[i] == '.'){
            for(j = i+1; j < strlen(path); j++) {
                sprintf(content_type, "%s%c", content_type, path[j]);
            }
            break;
        }
    }

    if(strcmp(content_type, "txt") == 0){
        strcpy(content_type, "text/plain; charset=utf-8");
    }
    else if(strcmp(content_type, "jpg") == 0){
        strcpy(content_type, "image/jpg");
    }
    else if(strcmp(content_type, "jpeg") == 0){
        strcpy(content_type, "image/jpeg");
    }
    else if(strcmp(content_type, "png") == 0){
        strcpy(content_type, "image/png");
    }
    else if(strcmp(content_type, "gif") == 0){
        strcpy(content_type, "image/gif");
    }
    else if(strcmp(content_type, "mpeg") == 0){
        strcpy(content_type, "video/mpeg");
    }
    else if(strcmp(content_type, "webm") == 0){
        strcpy(content_type, "video/webm");
    }
    else if(strcmp(content_type, "html") == 0){
        strcpy(content_type, "text/html; charset=utf-8");
    }
    else if(strcmp(content_type, "css") == 0){
        strcpy(content_type, "text/css; charset=utf-8");
    }
    else if(strcmp(content_type, "pdf") == 0){
        strcpy(content_type, "application/pdf");
    }
    else if(strcmp(content_type, "doc") == 0 || strcmp(content_type, "dot") == 0){
        strcpy(content_type, "application/msword");
    }
    else if(strcmp(content_type, "docx") == 0){
        strcpy(content_type, "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    }
    else if(strcmp(content_type, "xls") == 0 || strcmp(content_type, "xlt") == 0){
        strcpy(content_type, "application/vnd.ms-excel");
    }
    else if(strcmp(content_type, "xlsx") == 0){
        strcpy(content_type, "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    }
    else if(strcmp(content_type, "ppt") == 0 || strcmp(content_type, "pot") == 0){
        strcpy(content_type, "application/vnd.ms-powerpoint");
    }
    else if(strcmp(content_type, "pptx") == 0){
        strcpy(content_type, "application/vnd.openxmlformats-officedocument.presentationml.presentation");
    }
    else if(strcmp(content_type, "php") == 0){
        strcpy(content_type, "text/plain; charset=utf-8");
        script = PHP;
    }
    else if(strcmp(content_type, "py") == 0){
        strcpy(content_type, "text/plain; charset=utf-8");
        script = PY;
    }
    else {
        fprintf(stdout, "Error Peticion: %s\n", ERROR_500);
        return 500;
    }

    // Obtenemos el Date 
    strftime(date, MAXBUF*sizeof(char), "%a, %d %b %Y %H:%M:%S %Z", &tm);

    // Obtenemos el Last-modified del recurso
    stat(full_path, &recurso);
    tm2 = *gmtime(&(recurso.st_mtime));
    strftime(last_modified, MAXBUF*sizeof(char), "%a, %d %b %Y %H:%M:%S %Z", &tm2);

    // Obtenemos el Content-length
    if(script == PY || script == PHP) conf->content_length = MAXBUF + 2;
    else conf->content_length = recurso.st_size + 2;

    sprintf(cabecera, "HTTP/1.%d %s\r\nHost: %s:%d\r\nServer: %s\r\nDate: %s\r\nLast-modified: %s\r\nContent-length: %ld\r\nContent-type: %s\r\n\r\n", 
            conf->http_version, OK_200, conf->ip_server, conf->num_puerto, conf->signature, date, last_modified, conf->content_length, content_type);

    // Liberamos buffers 
    libera_buffer(full_path);
    libera_buffer(content_type);
    libera_buffer(date);
    libera_buffer(last_modified);

    // Cerramos el fichero
    fclose(f);

    return script;
}


void procesa_options(char* cabecera, config* conf){
    char* date=crea_buffer(MAXBUF);
    time_t now = time(0);
    struct tm tm = *gmtime(&now);

    // Obtenemos el Date 
    strftime(date, MAXBUF*sizeof(char), "%a, %d %b %Y %H:%M:%S %Z", &tm);

    sprintf(cabecera, "HTTP/1.%d %s\r\nAllowed: GET, POST, OPTIONS\r\nHost: %s:%d\r\nServer: %s\r\nDate: %s\r\n", 
            conf->http_version, OK_200, conf->ip_server, conf->num_puerto, conf->signature, date);

    libera_buffer(date);
}

char* crea_buffer(int tam){
    char* buffer=NULL;

    buffer = (char*)calloc(tam, sizeof(char));
    if(!buffer) return NULL;

    return buffer;
}

void libera_buffer(char* buffer){
    if(buffer){
        free(buffer);
    }
}

void envia_por_trozos(int clientfd, char* buffer, char* path, char* path_args, char* body_args, config* conf, int tipo, int script){
    int bytesres;           // Bytes restantes por enviar
    int bytesRead = 0;      // Bytes leidos
    int bytesToRead = MSG;  // Bytes a leer
    FILE* f=NULL;           // Recurso solicitado
    char* full_path=NULL;   // Path completo
    char* command=NULL;     // Comando script

    // Reservamos buffers
    full_path = crea_buffer(MAXBUF);

    // Inicializamos path
    strcpy(full_path, "./");

    // Buscamos y abrimos el recurso solicitado
    sprintf(full_path, "%s%s%s", full_path, conf->server_route, path);
    
    // Abrimos el recurso (script o normal)
    if(script == PHP){
        command = crea_buffer(MAXBUF);
        sprintf(command, "php %s", full_path);
        // Obtiene e introduce los argumentos al comando (si hay)
        if(tipo == GET){
            if(path_args){
                obtiene_args_path(command, path_args);
            }
        }
        else if(tipo == POST){
            obtiene_args_path(command, body_args);
        }
        printf("----Ejecutado----\n  %s\n\n\n", command);
        // Abre descriptor y ejecuta el comando
        f = popen(command, "r");
    }
    else if(script == PY){
        command = crea_buffer(MAXBUF);
        sprintf(command, "python %s", full_path);
        // Obtiene e introduce los argumentos al comando (si hay)
        if(tipo == GET){
            if(path_args){
                obtiene_args_path(command, path_args);
            }
        }
        else if(tipo == POST){
            obtiene_args_path(command, body_args);
        }
        printf("----Ejecutado----\n%s\n\n\n", command);
        // Abre descriptor y ejecuta el comando
        f = popen(command, "r");
    }
    else{
        f = fopen(full_path, "rb");  
    }
    if(!f){
        perror("Error abriendo el recurso");
        return;
    }

    if(script != PY  && script != PHP){
        // Inicializamos bytes por leer
        bytesres = conf->content_length;

        // Leemos y enviamos recurso en cachos
        while ((bytesRead = fread(buffer, 1, bytesToRead, f)) > 0){
            if(bytesres <= 0){
                break;
            }
            send(clientfd, buffer, bytesRead, 0);
            bytesres -= bytesRead;
            if(bytesres < MSG) bytesToRead = bytesres;
            // printf("Bytes ->>>> %d\n", bytesRead);
        }
    }
    else{
        strcpy(command, "");
        fread(command, 1, MAXBUF, f);
        send(clientfd, command, strlen(command), 0);
        libera_buffer(command);
    }

    // Liberamos recursos
    libera_buffer(full_path);
    if(script == PHP || script == PY) pclose(f);
    else fclose(f);
} 

void obtiene_args_path(char* command, char* path_args){
    int i=0, out=0;

    for(i = 0; i < MAXBUF && out == 0; i++){
        if(path_args[i] == '\n'){
            break;
        }
        else if(path_args[i] == '='){
            sprintf(command, "%s%c", command, ' ');
            for(i = i + 1; i < MAXBUF; i++){
                if(path_args[i] == '\n'){
                    out = 1;
                    break;
                }
                else if(path_args[i] == ' ' || path_args[i] == '&'){
                    break;
                }
                else if(path_args[i] == '+'){
                    sprintf(command, "%s%c", command, ' ');
                }
                else{
                    sprintf(command, "%s%c", command, path_args[i]);
                }
            }
        }
    }

}

void send_error_html(int clientfd, int error, char* cabecera, config* conf){
    char* contenido=NULL;
    char* fin_msg=NULL;
    char* full_path=NULL;
    char* date=NULL;
    char* last_modified=NULL;
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    struct tm tm2;
    struct stat recurso;
    FILE* f=NULL;

    // Inicializamos path
    full_path = crea_buffer(MAXBUF);
    strcpy(full_path, "./");

    // Buscamos y abrimos el recurso solicitado segun tipo de error
    if(error == 400){
        sprintf(full_path, "%s%s%s", full_path, conf->server_route, "/error_400.html");
    }
    else if(error == 404){
        sprintf(full_path, "%s%s%s", full_path, conf->server_route, "/error_404.html");
    }
    else if(error == 500){
        sprintf(full_path, "%s%s%s", full_path, conf->server_route, "/error_500.html");
    }

    // Abrimos recurso para obtener info
    f = fopen(full_path, "rb");
    if(!f){
        libera_buffer(full_path);
        return;
    }

    // Obtenemos el Date 
    strftime(date, MAXBUF*sizeof(char), "%a, %d %b %Y %H:%M:%S %Z", &tm);

    // Obtenemos el Last-modified del recurso
    stat(full_path, &recurso);
    tm2 = *gmtime(&(recurso.st_mtime));
    strftime(last_modified, MAXBUF*sizeof(char), "%a, %d %b %Y %H:%M:%S %Z", &tm2);

    // Obtenemos el Content-length
    conf->content_length = recurso.st_size + 2;

    // Creamos cabecera segun tipo de error
    if(error == 400){
        sprintf(cabecera, "HTTP/1.%d %s\r\nHost: %s:%d\r\nServer: %s\r\nDate: %s\r\nLast-modified: %s\r\nContent-length: %ld\r\nContent-type: %s\r\n\r\n", 
        conf->http_version, ERROR_400, conf->ip_server, conf->num_puerto, conf->signature, date, last_modified, conf->content_length, "text/html; charset=utf-8");
    }
    else if(error == 404){
        sprintf(cabecera, "HTTP/1.%d %s\r\nHost: %s:%d\r\nServer: %s\r\nDate: %s\r\nLast-modified: %s\r\nContent-length: %ld\r\nContent-type: %s\r\n\r\n", 
        conf->http_version, ERROR_404, conf->ip_server, conf->num_puerto, conf->signature, date, last_modified, conf->content_length, "text/html; charset=utf-8");
    }
    else if(error == 500){
        sprintf(cabecera, "HTTP/1.%d %s\r\nHost: %s:%d\r\nServer: %s\r\nDate: %s\r\nLast-modified: %s\r\nContent-length: %ld\r\nContent-type: %s\r\n\r\n", 
        conf->http_version, ERROR_500, conf->ip_server, conf->num_puerto, conf->signature, date, last_modified, conf->content_length, "text/html; charset=utf-8");
    }

    // Envia cabecera de error
    write(clientfd, cabecera, strlen(cabecera));

    contenido = crea_buffer(MAXBUF);

    // Enviamos el contenido con el html de error
    if(error == 400){
        envia_por_trozos(clientfd, contenido, "/error_400.html", NULL, NULL, conf, 0, 0);
    }
    else if(error == 404){
        envia_por_trozos(clientfd, contenido, "/error_404.html", NULL, NULL, conf, 0, 0);
    }
    else if(error == 500){
        envia_por_trozos(clientfd, contenido, "/error_500.html", NULL, NULL, conf, 0, 0);
    }

    fin_msg = crea_buffer(MSG);
    strcpy(fin_msg, "\r\n\r\n");
    send(clientfd, fin_msg, strlen(fin_msg), 0);

    libera_buffer(full_path);
    libera_buffer(contenido);
    libera_buffer(fin_msg);
}
