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
    //Primero creo un handler para el socket del servidor
    if(server_fd = socket(AF_INET, SOCK_STREAM, 0) == 0){//@TODO: Eliminar el == 0
        //Ocurrió un error al crear el socket
        perror("Socket creation error");
        return EXIT_FAILURE;
        
    }
    return (EXIT_SUCCESS);
}

