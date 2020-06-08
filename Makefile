# File: Makefile
# DanMat27
# AMP

CC = gcc
CFLAGS = -pthread
OBS = ./obj/server.o ./obj/server_functions.o ./obj/picohttpparser.o
EXE = server


#ALL
all: mkdir_obj server clear rmdir_obj


#EJECUTABLES
server: $(OBS) ./lib/libthreads.a
	$(CC) $(CFLAGS) -o $(EXE) $(OBS) ./lib/libthreads.a


#OBJETOS
./obj/server.o: ./src/server.c
	$(CC) $(CFLAGS) -o ./obj/server.o -c ./src/server.c

./obj/server_functions.o : ./src/server_functions.c
	$(CC) $(CFLAGS) -o ./obj/server_functions.o -c ./src/server_functions.c

./obj/picohttpparser.o: ./src/picohttpparser.c
	$(CC) $(CFLAGS) -o ./obj/picohttpparser.o -c ./src/picohttpparser.c 
	
#LIBRERIAS
./lib/libthreads.a: ./src/threads.c
	mkdir -p lib
	$(CC) $(CFLAGS) -o ./obj/threads.o -c ./src/threads.c
	ar -rcs ./lib/libthreads.a ./obj/threads.o

#LIMPIAR EJECUTABLES, OBJETOS Y OTROS
clean:
	rm -rf $(OBS) $(EXE)

#LIMPIAR OBJETOS
clear:
	rm -rf $(OBS)


#CREA DIRECTORIO OBJ PARA OBJETOS
mkdir_obj:
	mkdir -p obj

#BORRA DIRECTORIO OBJ 
rmdir_obj:
	rm -r obj


#AYUDA DE COMANDOS MAKEFILE
help:
	@echo "Comandos disponibles:"
	@echo "	make help  : Ayuda de compilacion"
	@echo "	make       : Compilacion de todo el servidor"
	@echo "	make all   : Compilacion de todo el servidor"
	@echo "	make clean : Limpiar ejecutables, objetos y otros"
	@echo "	make clear : Limpiar objetos"
	@echo "	make mkdir_obj : Crea carpeta obj para los objetos"
	@echo "	make rmdir_obj : Borra la carpeta obj"