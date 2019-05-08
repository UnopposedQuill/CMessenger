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
//#define SERVER_PORT 15000
//#define CLIENT_PORT 15001

//#define MAX_WAITING_CONNECTIONS 3

//También el tamaño del buffer
#define BUFFER_SIZE 1024

//El buffer de entrada y salida estándar
#define IO_BUFFER_SIZE 256

//Finalmente la dirección en la que supuestamente está el servidor, también se cambiará desde el .ini
//#define SERVER_ADDRESS "127.0.0.1"

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
    int server_fd, socket_handler, valread;

    FILE *archivo;
    int puertoCliente, puertoServidor, maxWaitingConnections;
    char direccionServidor[64] = "127.0.0.1";
    
    archivo = fopen("configFile.ini","r");
    if (archivo == NULL){
        printf("\nError de apertura del archivo. Utilizando datos por defecto\n\n");
        puertoCliente = 15001;
        puertoServidor = 15000;
        maxWaitingConnections = 3;
    }
    else{
        fscanf(archivo, 
                "[SETUP]\n"
                "Port=%d\n"
                "ServerPort=%d\n"
                "ServerAddress=%s\n"
                "MaxWaitingConnections=%d",
                &puertoCliente, &puertoServidor, direccionServidor, &maxWaitingConnections);
        //printf("%d\n", puertoCliente);
        fclose(archivo);
        printf("Archivo de inicialización leído correctamente\n");
    }
    

    //El descriptor que se usarán para la tubería entre ambos procesos del fork
    int descriptor[2];
    char bufferTuberia[BUFFER_SIZE];

    //Creo la tubería
    pipe(descriptor);

    //La dirección del socket del servidor
    struct sockaddr_in serv_addr;

    //El buffer que contendrá los datos que se le enviarán al servidor
    char data[BUFFER_SIZE];
    memset(data, 0, BUFFER_SIZE);

    //El buffer que contendrá los datos de IO
    char ioBuffer[IO_BUFFER_SIZE];
    
    printf("Bienvenido al cliente de CMessenger.\n\n1. Registrarse\n2. Iniciar Sesión\n");
    int opcion;
    fgets(ioBuffer, IO_BUFFER_SIZE, stdin);
    int datosMenu = sscanf(ioBuffer, "%d", &opcion);
    if(datosMenu <= 0){
        perror("Invalid Selection");
        return EXIT_FAILURE;
    }

    printf("Ingrese su nombre de usuario (Máximo 20 caracteres): ");
    char nombreUsuario[21];

    fgets(ioBuffer, 21, stdin);
    datosMenu = sscanf(ioBuffer, "%s", nombreUsuario);
    if(strlen(ioBuffer) <= 0){
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
        //Ahora el puerto: puertoServidor
        serv_addr.sin_port = htons(puertoServidor);

        //Ahora necesito convertir las direcciones a su forma binaria:
        if(inet_pton(AF_INET, direccionServidor, &serv_addr.sin_addr) <= 0){
            perror("Address invalid or not supported");
            return EXIT_FAILURE;
        }

        //Ahora intento conectar con el servidor
        if(connect(socket_handler, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
            perror("Couldn't connect to server");
            return EXIT_FAILURE;
        }

        //Ahora la información que agrego es: el tipo de servicio, el puertoCliente en el que escucharé y el nombre del usuario
        snprintf(data, 2 + cantidadDigitos(puertoCliente), "0%d\0", puertoCliente);
        strncat2(data, nombreUsuario, strlen(nombreUsuario));

        //Ahora intento escribirle los datos
        if((valread = send(socket_handler, data, 2 + cantidadDigitos(puertoCliente) + strlen(nombreUsuario), 0)) < 0){
            perror("Couldn't write data to server");
            return EXIT_FAILURE;
        }

        //Ahora deseo recibir la confirmación del servidor
        if((valread = recv(socket_handler, data, 1, 0)) < 0){
            perror("Couldn't response data from server");
            return EXIT_FAILURE;
        }

        if(data[0] == '1'){
            printf("Correct insertion of new user\n");
        }
        else{
            printf("Failure upon insertion of new user\n");
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
        //Ahora el puerto: puertoServidor
        serv_addr.sin_port = htons(puertoServidor);

        //Ahora necesito convertir las direcciones a su forma binaria:
        if(inet_pton(AF_INET, direccionServidor, &serv_addr.sin_addr) <= 0){
            perror("Address invalid or not supported");
            return EXIT_FAILURE;
        }

        //Ahora intento conectar con el servidor
        if(connect(socket_handler, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
            perror("Couldn't connect to server");
            return EXIT_FAILURE;
        }

        //Ahora la información que agrego es: el tipo de servicio, el puertoCliente en el que escucharé y el nombre del usuario
        snprintf(data, 2 + cantidadDigitos(puertoCliente), "2%d\0", puertoCliente);
        strncat2(data, nombreUsuario, strlen(nombreUsuario));

        if((valread = send(socket_handler, data, 2 + cantidadDigitos(puertoCliente) + strlen(nombreUsuario), 0)) < 0){
            perror("Couldn't write data to server");
            return EXIT_FAILURE;
        }

        //Ahora deseo recibir la confirmación del servidor
        if((valread = recv(socket_handler, data, 1, 0)) < 0){
            perror("Couldn't response data from server");
            return EXIT_FAILURE;
        }

        if(data[0] == '1'){
            printf("Correct login of new user\n");
        }
        else{
            printf("Failure upon login of new user\n");
            return EXIT_FAILURE;
        }
        close(socket_handler);

    }else{
        printf("Invalid Selection\n");
        return EXIT_FAILURE;
    }
    
    struct ListaContactos contactos = {NULL};
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
        address.sin_port = htons(puertoCliente);
        
        //Primero creo un handler para el socket del servidor
        if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){//@TODO: Eliminar el == 0
            //Ocurrió un error al crear el socket
            perror("Socket creation error");
            return EXIT_FAILURE;
        }
        
        
        //Creación del socket exitosa, ahora lo que hago es que señalo al SO que reutilice la ip
        if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
            perror("SO couldn't be instructed to reuse the socket address");
            return EXIT_FAILURE;
        }
        
        int cantidadFallos = 0;
        
        //Ahora ya uniré el socket y la dirección, necesito el handler del socket, la dirección
        //a unir y finalmente la longitud de la dirección
        while(cantidadFallos < 3 && (bind(server_fd, (struct sockaddr*) &address, (socklen_t) addrlen) < 0)){
            perror("Socket couldn't be bound to specified address, port or protocols");
            address.sin_port = htons(++puertoCliente);
            cantidadFallos++;
        }
        if(cantidadFallos >= 3){
            perror("Too many retries to set the listening port");
            return EXIT_FAILURE;
        }
        
        printf("Puerto seleccionado: %d\n", puertoCliente);

        //Ahora falta señalarle al SO que va a escuchar del puertoCliente, con un máximo de conexiones pendientes
        if(listen(server_fd, maxWaitingConnections) < 0){
            perror("Socket couldn't be signaled to listen");
            return EXIT_FAILURE;
        }
        printf("Initialization Success, listening on port %d\n", puertoCliente);
        close(descriptor[LEER]);
        //debo asegurarme de actualizarlo
        snprintf(bufferTuberia, cantidadDigitos(puertoCliente)+1, "%d", puertoCliente);
        write(descriptor[ESCRIBIR], bufferTuberia, cantidadDigitos(puertoCliente));
        printf("All parameters have been notified\n");
        memset(data, 0, BUFFER_SIZE);
        memset(bufferTuberia, 0, BUFFER_SIZE);
        
        while(1){
            if((socket_handler = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0){
                //No pudo aceptarla
                perror("\nError upon accepting a new connection");
            }
            else{
                //Pudo aceptarla, intento leer los datos
                //Primero tengo que leer el primer byte, el cual me dirá qué acción tengo que tomar
                if((valread = recv(socket_handler, data, BUFFER_SIZE, 0)) < 0){
                    perror("Error upon reading new data");
                }
                else{
                    printf("New Message incoming\n");
                    buscador = data;
                    while(*(buscador++));
                    printf("Nuevo Mensaje Recibido:\n"
                            "Remitente: %s\n"
                            "Contenido: \n"
                            "%s\n", data, buscador);
                }
                memset(data, 0, BUFFER_SIZE);
                memset(bufferTuberia, 0, BUFFER_SIZE);
                close(socket_handler);
            }
        }
    //Es mayor a cero, es el proceso pariente, que va a desplegar el menú y funciones
    }
    else{
        close(descriptor[ESCRIBIR]);
        memset(bufferTuberia, 0, BUFFER_SIZE);
        //Hasta que el hijo no le notifique cuál puertoCliente seleccionó, no avanza
        while((read(descriptor[LEER], bufferTuberia, BUFFER_SIZE)) < 0);
        sscanf(bufferTuberia, "%d", &puertoCliente);
        //Ahora intentaré hacer el socket nuevo para poder enviarlo al servidor
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
        //Ahora el puerto: puertoServidor
        serv_addr.sin_port = htons(puertoServidor);

        //Ahora necesito convertir las direcciones a su forma binaria:
        if(inet_pton(AF_INET, direccionServidor, &serv_addr.sin_addr) <= 0){
            perror("Address invalid or not supported");
            return EXIT_FAILURE;
        }

        if(connect(socket_handler, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
            perror("Couldn't connect to server");
            return EXIT_FAILURE;
        }

        //Para esto reinicio a 0 todo el buffer
        memset(data, 0, BUFFER_SIZE);
        
        snprintf(data, BUFFER_SIZE, "5%d", puertoCliente);
        strncat2(data, nombreUsuario, strlen(nombreUsuario));
        if(send(socket_handler, data, 3 + cantidadDigitos(puertoCliente) + strlen(nombreUsuario), 0) < 0){
            perror("Couldn't write data to server");
            return EXIT_FAILURE;
        }
        else{
            printf("Datos enviados exitosamente, esperando respuesta...\n");
            if(recv(socket_handler, data, 1, 0) < 0){
                perror("Error while reading server's response");
                return EXIT_FAILURE;
            }
            else if(data[0] == '1'){
                printf("Datos de recepción configurados exitosamente\n");
                close(socket_handler);
            }
            else{
                printf("No se pudo configurar los datos de recepción\n");
                return EXIT_FAILURE;
            }
        }
        
        //int cantidadElementos = 0;
        char directiva = 0;
        
        while(directiva != '3'){
            //Este es el proceso padre, ahora mismo voy a crear un menú capaz de utilizar todas las funcionalidades del cliente
            memset(ioBuffer, 0, IO_BUFFER_SIZE);
            printf("\nBienvenido al cliente de CMessenger."
                    "\n"
                    "\n"
                    "1. Agregar Contacto\n"
                    "2. Enviar Mensaje\n"
                    "3. Cerrar Sesion y salir\n"
                    "4. Actualizar Datos de Sesión\n"
                    "5. Ver contactos\n");
            if(fgets(ioBuffer, 256, stdin) == NULL){
                perror("Directive reading error");
                return EXIT_FAILURE;
            }
            sscanf(ioBuffer, "%c", &directiva);
            
            switch(directiva){
                case '1':{
                    //Agregar un nuevo contacto, servicio 1 del servidor
                    
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
                    //Ahora el puerto: puertoServidor
                    serv_addr.sin_port = htons(puertoServidor);

                    //Ahora necesito convertir las direcciones a su forma binaria:
                    if(inet_pton(AF_INET, direccionServidor, &serv_addr.sin_addr) <= 0){
                        perror("Address invalid or not supported");
                        return EXIT_FAILURE;
                    }

                    if(connect(socket_handler, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
                        perror("Couldn't connect to server");
                        return EXIT_FAILURE;
                    }

                    //Para esto reinicio a 0 todo el buffer
                    memset(data, 0, BUFFER_SIZE);
                    printf("Digite el nombre del Usuario: ");
                    char nombreContacto[128];
                    fgets(ioBuffer, 127, stdin);
                    sscanf(ioBuffer, "%s", nombreContacto);
                    
                    snprintf(data, BUFFER_SIZE, "1%s", nombreUsuario);
                    strncat2(data, nombreContacto, strlen(nombreContacto));

                    if(send(socket_handler, data, 3 + strlen(nombreUsuario) + strlen(nombreContacto), 0) < 0){
                        perror("Couldn't write data to server");
                        return EXIT_FAILURE;
                    }
                    else{
                        printf("Datos enviados exitosamente, esperando respuesta...\n");
                        if(recv(socket_handler, data, 1, 0) < 0){
                            perror("Error while reading server's response");
                        }
                        else if(data[0] == '1'){
                            //tengo que agregar el contacto a este lado
                            nc = (struct NodoContactos *)calloc(1, sizeof(struct NodoContactos));
                            nc->nombreContacto = (char *)calloc(strlen(nombreContacto)+1, sizeof(char));
                            strncpy(nc->nombreContacto, nombreContacto, strlen(nombreContacto));
                            insertarContactoAlInicio(&contactos, nc);
                            printf("Contacto agregado exitosamente");
                        }
                        else{
                            printf("No se pudo agregar el contacto");
                        }
                    }
                    memset(data, 0, BUFFER_SIZE);
                    close(socket_handler);
                    break;
                }
                case '2':{
                    //Enviar Mensaje, servicio numero 4 del servidor
                    //Primero necesito el remitente, destinatario y el contenido, el remitente es conseguible a partir del nombre de usuario
                    char destinatario[128], contenido[256];
                    printf("Ingrese el destinatario: ");
                    fgets(ioBuffer, 128, stdin);
                    sscanf(ioBuffer, "%s", destinatario);
                    
                    //Ahora necesito que el destinatario exista
                    if(!existeContacto(&contactos, destinatario)){
                        printf("El destinatario no está en la lista de contactos, cancelando operación");
                        break;
                    }
                    //Existe en los contactos
                    printf("Ingrese el contenido del mensaje:\n");
                    fgets(ioBuffer, 256, stdin);
                    sscanf(ioBuffer, "%s", contenido);
                    
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
                    //Ahora el puerto: puertoServidor
                    serv_addr.sin_port = htons(puertoServidor);

                    //Ahora necesito convertir las direcciones a su forma binaria:
                    if(inet_pton(AF_INET, direccionServidor, &serv_addr.sin_addr) <= 0){
                        perror("Address invalid or not supported");
                        return EXIT_FAILURE;
                    }

                    if(connect(socket_handler, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
                        perror("Couldn't connect to server");
                        return EXIT_FAILURE;
                    }
                    
                    //Ahora paso los datos a data para enviarlos
                    strncpy(data, "4", 1);
                    strncat(data, nombreUsuario, strlen(nombreUsuario));
                    strncat2(data, destinatario, strlen(destinatario));
                    buscador = data;
                    while(*(buscador++));
                    while(*(buscador++));
                    strncpy(buscador, contenido, strlen(contenido));
                    
                    if(send(socket_handler, data, strlen(nombreUsuario)+strlen(destinatario)+strlen(contenido)+3, 0) < 0){
                        perror("Error while writing data to server");
                        break;
                    }
                    
                    memset(data,0, BUFFER_SIZE);
                    recv(socket_handler, data, 1, 0);
                    if(data[0] == '1'){
                        //Envío correcto
                        printf("El mensaje fue enviado\n");
                    }
                    else{
                        //Envío incorrecto
                        printf("El mensaje no fue enviado\n");
                    }
                    memset(data, 0, BUFFER_SIZE);
                    close(socket_handler);
                    break;
                }
                case '3':{
                    //Salir, no tengo que hacer nada más que este break
                    break;
                }
                case '4':{
                    //Actualizar la información de la sesión
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
                    //Ahora el puerto: puertoServidor
                    serv_addr.sin_port = htons(puertoServidor);

                    //Ahora necesito convertir las direcciones a su forma binaria:
                    if(inet_pton(AF_INET, direccionServidor, &serv_addr.sin_addr) <= 0){
                        perror("Address invalid or not supported");
                        return EXIT_FAILURE;
                    }

                    if(connect(socket_handler, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
                        perror("Couldn't connect to server");
                        return EXIT_FAILURE;
                    }

                    //Para esto reinicio a 0 todo el buffer
                    memset(data, 0, BUFFER_SIZE);

                    snprintf(data, BUFFER_SIZE, "5%d", puertoCliente);
                    strncat2(data, nombreUsuario, strlen(nombreUsuario));
                    if(send(socket_handler, data, 3 + cantidadDigitos(puertoCliente) + strlen(nombreUsuario), 0) < 0){
                        perror("Couldn't write data to server");
                        return EXIT_FAILURE;
                    }
                    else{
                        printf("Datos enviados exitosamente, esperando respuesta...\n");
                        if(recv(socket_handler, data, 1, 0) < 0){
                            perror("Error while reading server's response");
                        }
                        else if(data[0] == '1'){
                            printf("Datos de recepción configurados exitosamente\n");
                        }
                        else{
                            printf("No se pudo configurar los datos de recepción\n");
                        }
                    }
                    close(socket_handler);
                    
                    break;
                }
                case '5':{
                    //mostrar la lista de contactos, función interna del cliente
                    imprimirListaContactos(&contactos);
                    break;
                }
                case '\n':{
                    break;
                }
                default:{
                    printf("Didn't recognize said option, please retry\n");
                    break;
                }
            }
        }
        printf("Logging out...\n");
        //Usar servicio número 3 del servidor
        //Actualizar la información de la sesión
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
        //Ahora el puerto: puertoServidor
        serv_addr.sin_port = htons(puertoServidor);

        //Ahora necesito convertir las direcciones a su forma binaria:
        if(inet_pton(AF_INET, direccionServidor, &serv_addr.sin_addr) <= 0){
            perror("Address invalid or not supported");
            return EXIT_FAILURE;
        }

        if(connect(socket_handler, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
            perror("Couldn't connect to server");
            return EXIT_FAILURE;
        }

        //Para esto reinicio a 0 todo el buffer
        memset(data, 0, BUFFER_SIZE);
        
        snprintf(data, BUFFER_SIZE, "3%s", nombreUsuario);
        if(send(socket_handler, data, 1 + strlen(nombreUsuario), 0) < 0){
            perror("Couldn't write data to server");
            return EXIT_FAILURE;
        }
        else{
            printf("Datos enviados exitosamente, esperando respuesta...\n");
            if(recv(socket_handler, data, 1, 0) < 0){
                perror("Error while reading server's response");
            }
            else if(data[0] == '1'){
                printf("Cierre de sesión exitoso\n");
            }
            else{
                printf("No se pudo cerrar la sesión\n");
            }
        }
        printf("Wiping data...\n");
        limpiarContactos(&contactos);
        printf("Exiting...\n");
        return EXIT_SUCCESS;
    }
}
