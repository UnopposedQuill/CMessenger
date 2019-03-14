/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: esteban
 *
 * Created on 6 de marzo de 2019, 08:44 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

//Me interesa colocar el puerto que usaré como una macro para colocarla alrededor del archivo
#define PORT 8080
//También colocar la cantidad de conexiones a la espera
#define MAX_WAITING_CONNECTIONS 3
//Finalmente el tamaño del buffer
#define BUFFER_SIZE 1024
/*
 * 
 */
int main(int argc, char** argv) {

    /*
     * Primero creo los handles para los sockets que usaré:
     * server_fd: El socket principal del servidor
     * new_socket: El socket que intenta comunicarse con el servidor
     */
    int server_fd, new_socket, valread;
    int opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    //Primero creo un handler para el socket del servidor
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){//@TODO: Eliminar el == 0
        //Ocurrió un error al crear el socket
        perror("Socket creation error");
        return EXIT_FAILURE;
    }
    //Creación del socket exitosa, ahora lo que hago es que señalo al SO que reutilice el puerto y la ip
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        perror("SO couldn't be instructed to reuse the socket address and port");
        return EXIT_FAILURE;
    }
    
    //En este punto ya es seguro colocar la ip, puertos y protocolos de los que deseo leer
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    //Ahora ya uniré el socket y la dirección, necesito el handler del socket, la dirección
    //a unir y finalmente la longitud de la dirección
    if(bind(server_fd, (struct sockaddr*) &address, (socklen_t) addrlen) < 0){
        perror("Socket couldn't be bound to specified address, port or protocols");
        return EXIT_FAILURE;
    }
    
    //Ahora falta señalarle al SO que va a escuchar del puerto, con un máximo de conexiones pendientes
    if(listen(server_fd, MAX_WAITING_CONNECTIONS) < 0){
        perror("Socket couldn't be signaled to listen");
        return EXIT_FAILURE;
    }
    printf("Initialization Success, listening on port %d\n", PORT);
    //Por siempre:
    while(1){
        //Intento aceptar una nueva conexión
        if((new_socket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0){
            //No pudo aceptarla
            perror("Error al aceptar un socket nuevo");
        }
        else{
            //Pudo aceptarla, intento leer los datos
            if((valread = read(new_socket, buffer, BUFFER_SIZE)) < -1){
                perror("Error al intentar leer los datos");
            }
            else{
                //en este momento valread contiene la cantidad de bytes leídos
                //Por ahora sólo le ordenaré al servidor imprimir todo lo que lee
                printf("%s\n", buffer);
            }
        }
    }
    
    return (EXIT_SUCCESS);
}

