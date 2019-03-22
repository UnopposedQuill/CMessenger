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

#define MAX_WAITING_CONNECTIONS 3

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
    int descriptorAHijo[2];
    int descriptorAPadre[2];
    char bufferTuberia[BUFFER_SIZE];

    //Creo las tuberías
    pipe(descriptorAHijo);
    pipe(descriptorAPadre);

    //La dirección del socket del servidor
    struct sockaddr_in serv_addr;

    //El manejador del socket, y una variable que guardará la cantidad de bytes que de verdad se leen
    int socket_handler = 0;

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
    
    struct ListaMensajes mensajes = {NULL};
    struct ListaContactos contactos = {NULL};
    
    struct NodoMensaje * nm;
    struct NodoContactos * nc;
    
    char * buscador;

    //Realizo un fork, y guardo el retorno en pid
    int pid = fork();
    //Si es menor a cero, no pudo realizarlo, error
    if(pid < 0){
        return EXIT_FAILURE;
    }
    //Si es cero, entonces es el subproceso, es el que va a estar recibiendo los nuevos mensajes
    else if(pid == 0){
        int opt = 1;
        
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(puerto);
        
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
        printf("Initialization Success, listening on port %d\n", puerto);
        close(descriptorAHijo[ESCRIBIR]);
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
                else{
                    nm = (struct NodoMensaje *) calloc(struct NodoMensaje);
                    buscador = data;
                    nm->mensaje->remitente = buscador;
                    while(*(buscador++));
                    nm->mensaje->destinatario = buscador;
                    while(*(buscador++));
                    nm->mensaje->contenido = buscador;
                    insertarMensajeAlInicio(&mensajes, nm);
                }
                if((valread = recv(descriptorAHijo[LEER], data, BUFFER_SIZE, 0)) > 0){
                    strncpy(data, nm->mensaje->remitente, strlen(nm->mensaje->remitente));
                    strncat2(data, nm->mensaje->destinatario, strlen(nm->mensaje->remitente));
                    strncat2(data, nm->mensaje->contenido, strlen(nm->mensaje->contenido));
                    write(descriptorAPadre[ESCRIBIR], data, strlen2(data));
                }
            }
        }
    //Es mayor a cero, es el proceso pariente, que va a desplegar el menú y funciones
    }
    else{
        close(descriptorAPadre[ESCRIBIR]);
        char directiva;
        while(1){
            //Este es el proceso padre, ahora mismo voy a crear un menú capaz de utilizar todas las funcionalidades del cliente
            scanf("%1d", &directiva);
            
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
