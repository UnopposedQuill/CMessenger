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
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "estructuras.h"

//Me interesa definir este puerto aquí
#define PORT 15000

//También el tamaño del buffer
#define BUFFER_SIZE 1024

//Finalmente la dirección en la que supuestamente está el servidor
#define SERVER_ADDRESS "127.0.0.1"
/*
 *
 */
int main(int argc, char** argv) {

    //Realizo un fork, y guardo el retorno en pid
    int pid = fork();
    //Si es menor a cero, no pudo realizarlo, error
    if(pid < 0){
        return EXIT_FAILURE;
    }
    //Si es cero, entonces es el subproceso, es el que va a estar recibiendo los nuevos mensajes
    else if(pid == 0){
        //por el momento no colocaré nada aquí
        return EXIT_SUCCESS;
    }
    //Es mayor a cero, es el proceso pariente, que va a desplegar el menú y funciones
    else{
        //Usaré esto para probar el sistema de envío de mensajes

        //La dirección del socket que va a leer
        struct sockaddr_in address;

        //La dirección del socket del servidor
        struct sockaddr_in serv_addr;

        //El manejador del socket, y una variable que guardará la cantidad de bytes que de verdad se leen
        int socket_handler = 0, valread;

        //Los datos que se le enviarán al servidor
        char data[1024] = "0Prueba";
        
        //Ahora intentaré hacer el socket nuevo
        if((socket_handler = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("Socket creation error");
            return EXIT_FAILURE;
        }
        //Memset se encarga de asignar un valor a la memoria, en este caso, la
        //idea es limpiar los valores de <serv_addr> que no necesito
        memset(&serv_addr, '0', sizeof(serv_addr));

        //Ahora coloco los valores que sí necesito
        //Primero el protocolo: Ipv4
        serv_addr.sin_family = AF_INET;
        //Ahora el puerto: PORT
        serv_addr.sin_port = htons(PORT);

        //Ahora necesito convertir las direcciones a su forma binaria:
        if(inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0){
            perror("Address invalid or not supported");
            return EXIT_FAILURE;
        }

        //Ahora intento conectar con el servidor
        if(connect(socket_handler, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
            perror("Couldn't connect to server");
            return EXIT_FAILURE;
        }
        
        //Ahora la segunda parte del mensaje, para esto me aseguro de que el buffer esté limpio
        memset(data, 0, BUFFER_SIZE);
        //Luego le agrego la siguiente información, son dos bytes que le dicen al servidor en qué puerto está escuchando el cliente
        snprintf(data, 8 + sizeof(int),"0%dPrueba", PORT);
        
        //Ahora intento escribirle la segunda parte de los datos
        if((valread = send(socket_handler, data, 8 + sizeof(int), 0)) < 0){
            perror("Couldn't write data to server");
            return EXIT_FAILURE;
        }
        
        printf("Data sent successfully, total bytes sent: %d\n", valread);
        
        //La dirección fue traducida correctamente
        return EXIT_SUCCESS;
    }
}
