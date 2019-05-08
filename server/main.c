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

//Esto es necesario para poder usar las señales de Linux
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>

//Las estructuras del servidor
#include "estructuras.h"

//Algunas utilidades necesarias para enviar números serializados
#include "utils.h"

//Me interesa colocar el puerto que usaré como una macro para colocarla alrededor del archivo
//#define PORT 15000
//También colocar la cantidad de conexiones a la espera
//#define MAX_WAITING_CONNECTIONS 3
//Finalmente el tamaño del buffer
#define BUFFER_SIZE 1024

//Esta variable le da señal al servidor de cuándo debe estar corriendo o no: 0 para detenerse
static volatile int keepRunning = 1;

//Necesito un handler que maneje la SIGINT que es disparada cuando se escribe CTRL+C en la terminal
void intHandler(int signal){
    //simplemente le paso la señal al servidor de que pare
    keepRunning = 0;
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    FILE *archivo;
    int maximoConexionesPendientes, puertoServidor;
    
    archivo = fopen("configFile.ini","r");
    if (archivo == NULL){
        printf("\nError de apertura del archivo. Utilizando datos por defecto\n\n");
        puertoServidor = 15000;
        maximoConexionesPendientes = 0;
    }
    else{
        fscanf(archivo, 
                "[SETUP]\n"
                "Port=%d\n"
                "MaxWaitingConnections=%d\n", &puertoServidor, &maximoConexionesPendientes);
        //printf("%d\n", puertoCliente);
        fclose(archivo);
        printf("Archivo de inicialización leído correctamente\n");
    }
    
    //Primero crea los sitios donde guardará los datos del servidor
    struct ListaClientes clientes = {NULL};
    struct ListaMensajes mensajes = {NULL};
    
    //Ahora a señalarle al SO que use el handler para SIGINT
    struct sigaction act;
    act.sa_handler = intHandler;
    sigaction(SIGINT, &act, NULL);
    
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
    address.sin_port = htons(puertoServidor);
    
    //Ahora ya uniré el socket y la dirección, necesito el handler del socket, la dirección
    //a unir y finalmente la longitud de la dirección
    if(bind(server_fd, (struct sockaddr*) &address, (socklen_t) addrlen) < 0){
        perror("Socket couldn't be bound to specified address, port or protocols");
        return EXIT_FAILURE;
    }
    
    //Ahora falta señalarle al SO que va a escuchar del puerto, con un máximo de conexiones pendientes
    if(listen(server_fd, maximoConexionesPendientes) < 0){
        perror("Socket couldn't be signaled to listen");
        return EXIT_FAILURE;
    }
    printf("Initialization Success, listening on port %d\n", puertoServidor);
    
    //Este buscador lo uso para buscar cadenas dentro de BUFFER, sin tener que moverlo
    char * buscador;
    
    //Estos dos son para las operaciones del servidor dentro de la base de datos
    struct Cliente * c;
    struct NodoCliente * nc;
    
    //Para almacenar nombres de destinatarios de mensaje
    char * nombreDestinatario;
    
    //Para almacenar contenidos de mensaje
    char * contenido;
    
    //Para almacenar los mensajes
    struct Mensaje * m;
    struct NodoMensaje * nm;
    
    //Mientras deba seguir corriendo:
    while(keepRunning){
        //Intento aceptar una nueva conexión
        if((new_socket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0){
            //No pudo aceptarla
            perror("\nError upon accepting a new connection");
        }
        else{
            //Pudo aceptarla, intento leer los datos
            //Primero tengo que leer el primer byte, el cual me dirá qué acción tengo que tomar
            if((valread = read(new_socket, buffer, 1)) < -1){
                perror("Error upon reading new data");
            }
            else{
                //en este momento valread contiene la cantidad de bytes leídos
                //Por ahora sólo le ordenaré al servidor imprimir todo lo que lee
                char directiva = buffer[0] - '0';
                switch(directiva){
                    case 0:{
                        //tengo que registrar un nuevo usuario, primero creo las variables que guardarán su información
                        c = (struct Cliente *)calloc(1, sizeof(struct Cliente));
                        nc = (struct NodoCliente *)calloc(1, sizeof(struct NodoCliente));
                        //Asignar el cliente al nodo...
                        nc->cliente = c;

                        //ahora agrego la dirección de la cual me escribe el new_socket
                        //Primero creo un arreglo donde se guardará la información
                        //Uso sólo Ipv4, así que ocupo 16 caracteres para la ip, y uno para finalizar
                        c->ipRegistrada = (char *)calloc(17, sizeof(char));

                        //En este punto la dirección debería seguir guardada en la información que usé en el accept
                        strncpy(c->ipRegistrada, inet_ntoa(address.sin_addr), 16);

                        //Ahora intento leer el supuesto puerto desde el cual desea leer lo que le envíe
                        if((valread = read(new_socket, buffer, BUFFER_SIZE)) > 0){
                            c->puertoRegistrado = atoi(buffer);
                            //ahora tengo que buscar el supuesto nombre del usuario, este while me deja buscador justo
                            //donde termina el número del puerto
                            buscador = buffer;
                            while(*(buscador++));

                            c->nombreUsuario = (char *) calloc(1, valread - strlen(buffer) - 1);
                            c->contactos = (struct ListaClientes *) calloc(1, sizeof(struct ListaClientes));
                            strncpy(nc->cliente->nombreUsuario, buscador, valread - strlen(buffer) - 1);

                            insertarClienteAlInicio(&clientes, nc);
                            printf("Successful client insertion: %s\n", c->nombreUsuario);

                            //Ahora me falta notificar al cliente que su inserción fue exitosa
                            //Como no tiene datos que enviar, sólo le envío un caracter
                            strncpy(buffer, "1", 1);

                            if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                perror("Error while notifying success to client upon insertion");
                            }
                            else{
                                printf("Client insertion operation behaved normally\n");
                            }
                        }
                        else{
                            //ocurrió un error, libero todo
                            free(c);
                            free(nc);
                            perror("Failed client insertion\n");
                        }
                        memset(buffer, 0, BUFFER_SIZE);
                        break;
                    }
                    case 1:{
                        //tengo que registrar un nuevo contacto a un cliente ya existente
                        //Primero creo una variable que se encargará de guardar el nombre del supuesto contacto
                        //La cual supuestamente está luego de la string del nombre del cliente
                        if((valread = read(new_socket, buffer, BUFFER_SIZE)) > 0){
                            char * nombreContacto = buffer;

                            //Este while me deja la variable justo después del caracter nulo
                            while(*(nombreContacto++));

                            //Si ambos usuarios existen: Nótese que se puede agregar a sí mismo como contacto
                            if(existeCliente(&clientes,buffer) && existeCliente(&clientes, nombreContacto)){
                                //Entonces agrego el segundo como contacto del primero
                                //Ya en este punto puedo asegurar que buscar no retornará NULL

                                //Creo un nuevo nodo para el contacto
                                struct NodoCliente * nodoContacto = (struct NodoCliente *) calloc(1, sizeof(struct NodoCliente));
                                nodoContacto->cliente = buscar(&clientes, nombreContacto);
                                insertarClienteAlInicio(buscar(&clientes, buffer)->contactos, nodoContacto);

                                //Ahora le notifico que pude realizarlo
                                strncpy(buffer,"1", 1);
                                if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                    perror("Error while notifying success to client upon insertion");
                                }
                                else{
                                    printf("Contact insertion operation behaved normally\n");
                                }
                            }
                            else{
                                //Le notifico que hubo un error
                                snprintf(buffer, 1, "-1");
                                if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                    perror("Error while notifying failure to client upon insertion");
                                }
                                else{
                                    printf("Client insertion failure informed\n");
                                }
                            }
                        }
                        else{
                            perror("Error while reading new extra data upon contact insertion");
                        }
                        memset(buffer, 0, BUFFER_SIZE);
                        break;
                    }
                    case 2:{
                        //inicio de sesión
                        if((valread = recv(new_socket, buffer, BUFFER_SIZE, 0))> 0){
                            if(existeCliente(&clientes, buffer)){
                                //Sí existe el usuario, así que lo busco
                                c = buscar(&clientes, buffer);
                                //En este punto la dirección debería seguir guardada en la información que usé en el accept
                                strncpy(c->ipRegistrada, inet_ntoa(address.sin_addr), 16);

                                //Ahora intento leer el supuesto puerto desde el cual desea leer lo que le envíe
                                if((valread = recv(new_socket, buffer, sizeof(int), 0)) > 0){
                                    c->puertoRegistrado = atoi(buffer);
                                    printf("Successful client login\n");

                                    //Primero el header que dice login exitoso
                                    strncpy(buffer, "1",1);

                                    //Ahora me falta enviar al cliente todos sus datos
                                    //Primero tengo que enviar todos los datos de los contactos que tiene el usuario
                                    int cantidadElementosAEnviar = cantidadClientes(c->contactos);
                                    int cantidadIntentosFallidos = 0;
                                    snprintf(buffer, cantidadDigitos(cantidadElementosAEnviar), "%d", cantidadElementosAEnviar);

                                    if((valread = send(new_socket, buffer, 1 + cantidadDigitos(cantidadElementosAEnviar), 0)) > 0){
                                        //Voy a usar nc para recorrer toda la lista de contactos del cliente
                                        nc = c->contactos->primerNodo;
                                        while(nc != NULL){
                                            strncpy(buffer, nc->cliente->nombreUsuario, strlen(nc->cliente->nombreUsuario));
                                            if((valread = send(new_socket, buffer, strlen(buffer), 0)) > 0){
                                                nc = nc->siguiente;
                                            }
                                            else if(cantidadIntentosFallidos < 3){
                                                cantidadIntentosFallidos++;
                                            }
                                            else{
                                                perror("Gave up delivering data to client upon login");
                                                nc = nc->siguiente;
                                            }
                                        }
                                        printf("Contacts delivery successfull\n");
                                        cantidadElementosAEnviar = cantidadMensajesUsuario(&mensajes, c->nombreUsuario);

                                        nm = mensajes.primerNodo;
                                        while(nm != NULL){
                                            if(strcmp(c->nombreUsuario, nm->mensaje->destinatario) == 0){
                                                strncpy(buffer, nm->mensaje->remitente, strlen(nm->mensaje->remitente));
                                                strncat2(buffer, nm->mensaje->destinatario, strlen(nm->mensaje->destinatario));
                                                buscador = buffer;
                                                while(*(buscador++));
                                                while(*(buscador++));
                                                strncat2(buffer, nm->mensaje->contenido, strlen(nm->mensaje->contenido));
                                                if((valread = send(new_socket, buffer, strlen2(buffer), 0)) > 0){
                                                    nc = nc->siguiente;
                                                }
                                                else if(cantidadIntentosFallidos < 3){
                                                    cantidadIntentosFallidos++;
                                                }
                                                else{
                                                    perror("Gave up delivering data to client upon login");
                                                    nc = nc->siguiente;
                                                }
                                            }
                                            else{
                                                nc = nc->siguiente;
                                            }

                                        }
                                        printf("Client login operation behaved normally\n");
                                    }
                                }
                                else{
                                    //Ocurrió un error durante la lectura de datos
                                    perror("Error upon reading username that's logging in");
                                }
                            }
                            else{
                                printf("Login with an unexisting user\n");
                                //Le notifico que hubo un error
                                strncpy(buffer, "0", 1);
                                if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                    perror("Error while notifying failure to client upon login");
                                }
                                else{
                                    printf("Client login failure informed\n");
                                }
                            }
                        }
                        memset(buffer, 0, BUFFER_SIZE);
                        break;
                    }
                    case 3:{
                        //cierre de sesión
                        if(recv(new_socket, buffer, BUFFER_SIZE, 0) > 0){
                            if(existeCliente(&clientes, buffer)){
                                //Sí existe el usuario, así que lo busco
                                c = buscar(&clientes, buffer);

                                free(c->ipRegistrada);
                                c->ipRegistrada = NULL;
                                c->puertoRegistrado = -1;

                                //Ahora me falta notificar al cliente que su inicio de sesión fue exitoso
                                //Como no tiene datos que enviar, sólo le envío un caracter
                                snprintf(buffer, 1, "1");
                                if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                    perror("Error while notifying success to client upon login");
                                }
                                else{
                                    printf("Client logout operation behaved normally\n");
                                }
                            }
                            else{
                                printf("Logout with an unexisting user\n");
                                //Le notifico que hubo un error
                                snprintf(buffer, 1, "-1");
                                if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                    perror("Error while notifying failure to client upon logout");
                                }
                                else{
                                    printf("Client logout failure informed\n");
                                }
                            }
                        }
                        else{
                            perror("Error while reading logout data");
                        }
                        memset(buffer, 0, BUFFER_SIZE);
                        break;
                    }
                    case 4:{
                        //enviar un mensaje, sólo se permite enviar mensajes a elementos dentro de la
                        //lista de contactos
                        //inicio de sesión
                        printf("New message delivery request\n");
                        recv(new_socket, buffer, BUFFER_SIZE, 0);
                        if(existeCliente(&clientes, buffer)){
                            //Sí existe el usuario, así que lo busco
                            c = buscar(&clientes, buffer);
                            printf("Sender found\n");
                            //Esto almacenará el destinatario
                            nombreDestinatario = buffer;
                            while(*(nombreDestinatario++));

                            //Esto almacenará el contenido
                            contenido = nombreDestinatario;
                            while(*(contenido++));

                            //Si existe dentro de los contactos
                            if(existeCliente(c->contactos, nombreDestinatario)){
                                //Existe, procedo a almacenarlo, y luego ver si puedo enviarlo
                                //Primero el paquete
                                nm = (struct NodoMensaje *) calloc(1, sizeof(struct NodoMensaje));

                                //Luego el mensaje
                                m = (struct Mensaje *) calloc(1, sizeof(struct Mensaje));
                                m->remitente = (char *) calloc(strlen(buffer)+1, sizeof(char));
                                m->destinatario = (char *) calloc(strlen(nombreDestinatario)+1, sizeof(char));
                                m->contenido = (char *) calloc(strlen(contenido)+1, sizeof(char));

                                strncpy(m->remitente, buffer, strlen(buffer));
                                strncpy(m->destinatario, nombreDestinatario, strlen(nombreDestinatario));
                                strncpy(m->contenido, contenido, strlen(contenido));

                                //Coloco el mensaje en el paquete
                                nm->mensaje = m;

                                //Coloco el paquete en la base
                                insertarMensajeAlInicio(&mensajes, nm);
                                printf("Message insertion successfull\n");

                                //Ahora a notificar el envío del mensaje
                                strncpy(buffer, "1", 1);
                                if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                    perror("Error while notifying success to client upon delivering message");
                                }
                                else{
                                    printf("Client message delivery informed\n");
                                }

                                //Ahora procedo a ver si puedo enviar de una vez el mensaje
                                //Para eso busco el destinatario
                                c = buscar(c->contactos, nombreDestinatario);

                                //Si esto se cumple, entonces está disponible para enviar el mensaje
                                if(c->ipRegistrada != NULL && c->puertoRegistrado != -1){

                                    //La dirección del socket del servidor
                                    struct sockaddr_in client_addr;

                                    //El manejador del socket, y una variable que guardará la cantidad de bytes que de verdad se leen
                                    int socket_handler;

                                    //Ahora intentaré hacer el socket nuevo
                                    if((socket_handler = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                                        perror("Socket creation error");
                                        return EXIT_FAILURE;
                                    }
                                    //Memset se encarga de asignar un valor a la memoria, en este caso, la
                                    //idea es limpiar los valores de <serv_addr> que no necesito
                                    memset(&client_addr, '0', sizeof(client_addr));

                                    //Ahora coloco los valores que sí necesito
                                    //Primero el protocolo: Ipv4
                                    client_addr.sin_family = AF_INET;
                                    //Ahora el puerto: SERVER_PORT
                                    client_addr.sin_port = htons(c->puertoRegistrado);

                                    //Ahora necesito convertir las direcciones a su forma binaria:
                                    if(inet_pton(AF_INET, c->ipRegistrada, &client_addr.sin_addr) <= 0){
                                        perror("Address invalid or not supported");
                                    }
                                    else if(connect(socket_handler, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0){
                                        perror("Couldn't connect to server");
                                    }
                                    else{
                                        //Conexión al cliente exitosa, cargo los datos del mensaje al buffer
                                        strncpy(buffer, m->remitente, strlen(m->remitente));
                                        strncat2(buffer, m->contenido, strlen(m->contenido));
                                        if((valread = send(socket_handler, buffer, 1+strlen(m->remitente)+strlen(m->contenido),0)) > 0){
                                            printf("Success upon delivering message instantly\n");
                                        }
                                        else{
                                            perror("Error upon sending new message data to client");
                                        }
                                    }
                                }
                                else{
                                    printf("Message couldn't be delivered immediately: User is not online\n");
                                }
                            }
                            else{
                                //No existe, retornar error    
                                strncpy(buffer, "0", 1);
                                if((valread = send(new_socket, buffer, 2, 0)) < 0){
                                    perror("Error while notifying failure to client sending message");
                                }
                                else{
                                    printf("Client message delivery failure informed\n");
                                }
                            }
                        }
                        else{
                            printf("Unexisting user sending a message\n");
                            //Le notifico que hubo un error
                            strncpy(buffer, "0", 1);
                            if((valread = send(new_socket, buffer, 2, 0)) < 0){
                                perror("Error while notifying failure to unregistered client");
                            }
                            else{
                                printf("Unregistered user failure informed\n");
                            }
                        }
                        memset(buffer, 0, BUFFER_SIZE);
                        break;
                    }
                    case 5:{
                        //actualizar los datos de la sesión
                        if((valread = recv(new_socket, buffer, BUFFER_SIZE, 0)) < 0){
                            perror("Couldn't receive the data from the session update");
                        }
                        else{
                            int puerto;
                            sscanf(buffer, "%d", &puerto);
                            buscador = buffer;
                            while(*(buscador++));
                            if(existeCliente(&clientes, buscador)){
                                buscar(&clientes, buscador)->puertoRegistrado = puerto;
                                strncpy(buffer, "1", 1);
                            }
                            else{
                                strncpy(buffer, "0", 1);
                            }
                            send(new_socket, buffer, 1, 0);
                        }
                        memset(buffer, 0, BUFFER_SIZE);
                        break;
                    }
                    default:{
                        printf("Action not implemented yet");
                        break;
                    }
                }
                //Me interesa borrar todo del buffer
                memset(buffer, 0, BUFFER_SIZE);
                //printf("%s\n", buffer);
                
                //Ahora cierro el socket
                close(new_socket);
            }
        }
    }
    printf("Wiping data...\n");
    //Cierro el socket del servidor
    close(server_fd);
    //Por el momento aquí sólo se liberará la memoria del programa, pero eventualmente se desea
    //que los datos persistan en algún sitio.
    limpiarClientes(&clientes);
    limpiarMensajes(&mensajes);
    
    //Finalizo todo
    printf("Shutting down server...\n");
    
    return EXIT_SUCCESS;
}

