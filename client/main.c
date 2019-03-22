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
#include "utils.h"

//Me interesa definir estos puertos aquí, eventualmente se cambiarán por la lectura del archivo
#define SERVER_PORT 15000
//#define CLIENT_PORT 15001

//También el tamaño del buffer
#define BUFFER_SIZE 1024

//Finalmente la dirección en la que supuestamente está el servidor, también se cambiará desde el .ini
#define SERVER_ADDRESS "127.0.0.1"

//Esto es para el pipe que es necesario para comunicar ambos procesos
#define LEER 0
#define ESCRIBIR 1
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

    FILE *archivo;
	char caracter;
    char iniInformation[512];
    int puerto;

	archivo = fopen("configFile.ini","r");

	if (archivo == NULL)
        {
            printf("\nError de apertura del archivo. \n\n");
        }
        else
        {
            fscanf(archivo, "[SETUP]\nPort=%d", &puerto);
            printf("%d\n", puerto);
        }
            //printf("\nEl contenido del archivo de prueba es \n\n");
            /*while((caracter = fgetc(archivo)) != EOF)
	    {
		printf("%c",caracter);
	    }
        }
        */
        fclose(archivo);

    //Dos descriptores que se usarán para la tubería entre ambos procesos del fork
    int descriptor[2];
    char bufferTuberia[BUFFER_SIZE];

    //Creo la tubería
    pipe(descriptor);

    //Usaré esto para probar el sistema de envío de mensajes

    //La dirección del socket del servidor
    struct sockaddr_in serv_addr;

    //El manejador del socket, y una variable que guardará la cantidad de bytes que de verdad se leen
    int socket_handler = 0, valread;

    //El buffer que contendrá los datos que se le enviarán al servidor
    char data[BUFFER_SIZE];
    memset(data, 0, BUFFER_SIZE);

    printf("Bienvenido al cliente de CMessenger.\n\n1. Registrarse\n2. Iniciar Sesión\n");
    int opcion;
    int datosMenu = scanf("%d", &opcion);
    if(datosMenu <= 0){
        perror("Invalid Selection");
        return EXIT_FAILURE;
    }

    printf("Ingrese su nombre de usuario (Máximo 20 caracteres): ");
    char nombreUsuario[21];

    datosMenu = scanf("%20s", nombreUsuario);
    if(datosMenu <= 0){
        perror("Invalid input");
        return EXIT_FAILURE;
    }

    if(opcion == 1){
        printf("Registrar: %s\n",nombreUsuario);

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
        //Ahora el puerto: SERVER_PORT
        serv_addr.sin_port = htons(SERVER_PORT);

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

        //Ahora la información que agrego es: el tipo de servicio, el puerto en el que escucharé y el nombre del usuario
        snprintf(data, 2 + cantidadDigitos(puerto), "0%d\0", puerto);
        strncat2(data, nombreUsuario, strlen(nombreUsuario));

        //Ahora intento escribirle los datos
        if((valread = send(socket_handler, data, 2 + cantidadDigitos(puerto) + strlen(nombreUsuario), 0)) < 0){
            perror("Couldn't write data to server");
            return EXIT_FAILURE;
        }

        printf("Data sent successfully, total bytes sent: %d\n", valread);

        //Ahora deseo recibir la confirmación del servidor
        if((valread = recv(socket_handler, data, 1, 0)) < 0){
            perror("Couldn't response data from server");
            return EXIT_FAILURE;
        }

        if(data[0] == '1'){
            printf("Correct insertion of new user");
        }
        else{
            printf("Failure upon insertion of new user");
            return EXIT_FAILURE;
        }
        close(socket_handler);

    }else if(opcion == 2){
        printf("Iniciar Sesión: %s\n",nombreUsuario);

        //La dirección del socket del servidor
        struct sockaddr_in serv_addr;

        //El manejador del socket, y una variable que guardará la cantidad de bytes que de verdad se leen
        int socket_handler = 0, valread;

        //El buffer que contendrá los datos que se le enviarán al servidor
        char data[BUFFER_SIZE];
        memset(data, 0, BUFFER_SIZE);

        //El nombre del usuario para las pruebas

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
        //Ahora el puerto: SERVER_PORT
        serv_addr.sin_port = htons(SERVER_PORT);

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

        //Ahora la información que agrego es: el tipo de servicio, el puerto en el que escucharé y el nombre del usuario
        snprintf(data, 2 + cantidadDigitos(puerto), "2%d\0", puerto);
        strncat2(data, nombreUsuario, strlen(nombreUsuario));

        if((valread = send(socket_handler, data, 2 + cantidadDigitos(puerto) + strlen(nombreUsuario), 0)) < 0){
            perror("Couldn't write data to server");
            return EXIT_FAILURE;
        }

        printf("Data sent successfully, total bytes sent: %d\n", valread);

        //Ahora deseo recibir la confirmación del servidor
        if((valread = recv(socket_handler, data, 1, 0)) < 0){
            perror("Couldn't response data from server");
            return EXIT_FAILURE;
        }

        if(data[0] == '1'){
            printf("Correct login of new user");
        }
        else{
            printf("Failure upon login of new user");
            return EXIT_FAILURE;
        }
        close(socket_handler);

    }else{
        printf("Invalid Selection\n");
        return EXIT_FAILURE;
    }

    //Realizo un fork, y guardo el retorno en pid
    int pid = fork();
    //Si es menor a cero, no pudo realizarlo, error
    if(pid < 0){
        return EXIT_FAILURE;
    }
    //Si es cero, entonces es el subproceso, es el que va a estar recibiendo los nuevos mensajes
    else if(pid == 0){
     //por el momento no colocaré nada aquí
        while(1){
            if((new_socket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0){
                //No pudo aceptarla
                perror("\nError upon accepting a new connection");
            }
            else{
                //Pudo aceptarla, intento leer los datos
                //Primero tengo que leer el primer byte, el cual me dirá qué acción tengo que tomar
                if((valread = recv(new_socket, data, BUFFER_SIZE, 0)) < -1){
                    perror("Error upon reading new data");
                }
                write (descriptor[ESCRIBIR], data, strlen2(data));
            }
        }
    //Es mayor a cero, es el proceso pariente, que va a desplegar el menú y funciones
    }
    else{
        while(1){
        //Ahora me interesa intentar insertar un nuevo contacto dentro de la lista de contactos dentro del servidor
        //Tengo que reconectarme puesto que estas conexiones se crean por cada solicitud
        //Para esto tengo que crear un nuevo socket como si fuera otro programa por completo
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
            //Ahora el puerto: SERVER_PORT
            serv_addr.sin_port = htons(SERVER_PORT);

            //Ahora necesito convertir las direcciones a su forma binaria:
            if(inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0){
                perror("Address invalid or not supported");
                return EXIT_FAILURE;
            }

            if(connect(socket_handler, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
                perror("Couldn't connect to server");
                return EXIT_FAILURE;
            }

            //Para esto reinicio a 0 todo el buffer
            memset(data, 0, BUFFER_SIZE);
            snprintf(data, BUFFER_SIZE, "1%s", nombreUsuario);
            strncat2(data, nombreUsuario, strlen(nombreUsuario));

            if((valread = send(socket_handler, data, 1 + 2*(strlen(nombreUsuario)+1), 0)) < 0){
                perror("Couldn't write data to server");
                return EXIT_FAILURE;
            }

            printf("Data sent successfully, total bytes sent: %d\n", valread);

            printf("Bienvenido al cliente de CMessenger222.\n\n1. Agregar Contacto\n2. Cerrrar Sesión\n3. Ver Mensajes\n4. Enviar Mensaje\n5. Cerrar Sesion");
            int opcion;
            int datosMenu = scanf("%d", &opcion);
            if(datosMenu <= 0){
                perror("Invalid Selection");
                return EXIT_FAILURE;
            }

            printf("Ingrese su nombre de usuario (Máximo 20 caracteres): ");
            char nombreUsuario[21];

            datosMenu = scanf("%20s", nombreUsuario);
            if(datosMenu <= 0){
                perror("Invalid input");
                return EXIT_FAILURE;
            }

            }
                return EXIT_SUCCESS;
        }
}
