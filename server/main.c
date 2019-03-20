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
#define PORT 15000
//También colocar la cantidad de conexiones a la espera
#define MAX_WAITING_CONNECTIONS 3
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
    
    //Este buscador lo uso para buscar cadenas dentro de BUFFER, sin tener que moverlo
    char * buscador;
    
    //Estos dos son para las operaciones del servidor dentro de la base de datos
    struct Cliente * c;
    struct NodoCliente * nc;
    
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
                            while(*(buscador++));
                            strncpy(nc->cliente->nombreUsuario, buscador, valread - strlen(buffer) - 1);
                            
                            insertarClienteAlInicio(&clientes, nc);
                            printf("Successful client insertion\n");

                            //Ahora me falta notificar al cliente que su inserción fue exitosa
                            //Como no tiene datos que enviar, sólo le envío un caracter
                            snprintf(buffer, 1, "1");
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
                        break;
                    }
                    case 1:{
                        //tengo que registrar un nuevo contacto a un cliente ya existente
                        //Primero creo una variable que se encargará de guardar el nombre del supuesto contacto
                        //La cual supuestamente está luego de la string del nombre del cliente
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
                            snprintf(buffer, 1, "1");
                            if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                perror("Error while notifying success to client upon insertion");
                            }
                            else{
                                printf("Client insertion operation behaved normally\n");
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
                        break;
                    }
                    case 2:{
                        //inicio de sesión
                        if(existeCliente(&clientes, buffer)){
                            //Sí existe el usuario, así que lo busco
                            c = buscar(&clientes, buffer);
                            //En este punto la dirección debería seguir guardada en la información que usé en el accept
                            strncpy(c->ipRegistrada, inet_ntoa(address.sin_addr), 16);
                        
                            //Ahora intento leer el supuesto puerto desde el cual desea leer lo que le envíe
                            if((valread = read(new_socket, buffer, sizeof(int))) > 0){
                                c->puertoRegistrado = atoi(buffer);
                            }

                            printf("Successful client login\n");

                            //Ahora me falta notificar al cliente que su inicio de sesión fue exitoso
                            //Como no tiene datos que enviar, sólo le envío un caracter
                            snprintf(buffer, 1, "1");
                            if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                perror("Error while notifying success to client upon login");
                            }
                            else{
                                printf("Client login operation behaved normally\n");
                            }
                        }
                        else{
                            printf("Login with an unexisting user\n");
                            //Le notifico que hubo un error
                            snprintf(buffer, 1, "-1");
                            if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                perror("Error while notifying failure to client upon login");
                            }
                            else{
                                printf("Client login failure informed\n");
                            }
                        }
                    }
                    case 3:{
                        //cierre de sesión
                        if(existeCliente(&clientes, buffer)){
                            //Sí existe el usuario, así que lo busco
                            c = buscar(&clientes, buffer);
                            //En este punto la dirección debería seguir guardada en la información que usé en el accept
                            strncpy(c->ipRegistrada, inet_ntoa(address.sin_addr), 16);
                        
                            //Ahora intento leer el supuesto puerto desde el cual desea leer lo que le envíe
                            if((valread = read(new_socket, buffer, sizeof(int))) > 0){
                                c->puertoRegistrado = atoi(buffer);
                            }

                            printf("Successful client login\n");

                            //Ahora me falta notificar al cliente que su inicio de sesión fue exitoso
                            //Como no tiene datos que enviar, sólo le envío un caracter
                            snprintf(buffer, 1, "1");
                            if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                perror("Error while notifying success to client upon login");
                            }
                            else{
                                printf("Client login operation behaved normally\n");
                            }
                        }
                        else{
                            printf("Login with an unexisting user\n");
                            //Le notifico que hubo un error
                            snprintf(buffer, 1, "-1");
                            if((valread = send(new_socket, buffer, 1, 0)) < 0){
                                perror("Error while notifying failure to client upon login");
                            }
                            else{
                                printf("Client login failure informed\n");
                            }
                        }
                    }
                    default:{
                        printf("Action not implemented yet");
                        break;
                    }
                }
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

