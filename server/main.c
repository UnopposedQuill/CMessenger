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
#include "estructuras.h"

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
    insertarAlInicio(&clientes, (struct NodoCliente *) calloc(1, sizeof(struct NodoCliente)));
    
    struct Cliente c = {"Prueba","",0,NULL};
    clientes.primerNodo = (struct NodoCliente *) calloc(1, sizeof(struct NodoCliente));
    clientes.primerNodo->cliente = &c;
    imprimirListaClientes(&clientes);
    */
    
    
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
                        struct Cliente * c = (struct Cliente *)calloc(1, sizeof(struct Cliente));
                        struct NodoCliente * nc = (struct NodoCliente *)calloc(1, sizeof(struct NodoCliente));
                        //Asignar el cliente al nodo...
                        nc->cliente = c;
                        
                        //Finalmente conseguir el supuesto nombre del usuario
                        if((valread = read(new_socket, buffer, BUFFER_SIZE)) > 0){
                            //logró conseguirlo, creo un buffer para el nombre del cliente
                            char * nombreUsuario = (char *)calloc(valread, sizeof(char));
                            
                            //ahora copio la string recibida
                            strncpy(nombreUsuario, buffer, valread);
                            
                            //lo coloco en el cliente
                            nc->cliente->nombreUsuario = nombreUsuario;
                            insertarClienteAlInicio(&clientes, nc);
                            printf("Inserción de nuevo cliente exitosa\n");
                        }
                        else{
                            //ocurrió un error, libero todo
                            free(c);
                            free(nc);
                            perror("Inserción de nuevo cliente fallida\n");
                        }
                        break;
                    }
                    default:{
                        perror("Acción no implementada\n");
                        break;
                    }
                    
                }
                //printf("%s\n", buffer);
                
                //Ahora cierro el socket
                close(new_socket);
            }
        }
    }
    printf("Limpiando Variables...\n");
    //Cierro el socket del servidor
    close(server_fd);
    //Por el momento aquí sólo se liberará la memoria del programa, pero eventualmente se desea
    //que los datos persistan en algún sitio.
    limpiarClientes(&clientes);
    limpiarMensajes(&mensajes);
    
    //Finalizo todo
    printf("Finalizando Servidor...\n");
    
    return EXIT_SUCCESS;
}

