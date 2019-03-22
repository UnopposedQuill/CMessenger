/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   estructuras.c
 * Author: esteban
 * 
 * Created on 18 de marzo de 2019, 12:32 AM
 */

#include <string.h>
#include "estructuras.h"

//Funciones Lista de Clientes
void insertarClienteAlInicio(struct ListaClientes * self, struct NodoCliente * nc){
    nc->siguiente = self->primerNodo;
    self->primerNodo = nc;
    return;
}

void imprimirListaClientes(struct ListaClientes * self){
    struct NodoCliente * recorreNodos = self->primerNodo;
    while(recorreNodos != NULL){
        printf("Nombre de Usuario: %s\n", recorreNodos->cliente->nombreUsuario);
        recorreNodos = recorreNodos->siguiente;
    }
    return;
}

void limpiarClientes(struct ListaClientes* self){
    struct NodoCliente * recorreNodos = self->primerNodo;
    self->primerNodo = NULL;
    while(recorreNodos != NULL){
        
        //Primero tengo que liberar los contactos
        struct NodoCliente * recorreContactos = recorreNodos->cliente->contactos->primerNodo;
        struct NodoCliente * nodoALiberar;
        while(recorreContactos != NULL){
            nodoALiberar = recorreContactos;
            recorreContactos = recorreContactos->siguiente;
            free(nodoALiberar);
        }
        free(recorreNodos->cliente->nombreUsuario);
        free(recorreNodos->cliente->ipRegistrada);
        free(recorreNodos->cliente);
        nodoALiberar = recorreNodos;
        free(nodoALiberar);
        recorreNodos = recorreNodos->siguiente;
    }
    return;
}

int cantidadClientes(struct ListaClientes* self){
    struct NodoCliente * recorreNodos = self->primerNodo;
    int resultado = 0;
    while(recorreNodos != NULL){
        resultado++;
    }
    return resultado;
}

int existeCliente(struct ListaClientes * self, const char * nombreUsuario){
    struct NodoCliente * recorreNodos = self->primerNodo;
    while(recorreNodos != NULL){
        if(strcmp(recorreNodos->cliente->nombreUsuario, nombreUsuario) == 0){
            return 1;
        }
        recorreNodos = recorreNodos->siguiente;
    }
    return 0;
}

struct Cliente * buscar(struct ListaClientes * self, const char * nombreUsuario){
    struct NodoCliente * recorreNodos = self->primerNodo;
    while(recorreNodos != NULL){
        if(strcmp(recorreNodos->cliente->nombreUsuario, nombreUsuario) == 0){
            return recorreNodos->cliente;
        }
    }
    return NULL;
}

//Funciones Lista Mensajes
void insertarMensajeAlInicio(struct ListaMensajes * self, struct NodoMensaje* nm){
    nm->siguiente = self->primerNodo;
    self->primerNodo = nm;
    return;
}

void imprimirListaMensajes(struct ListaMensajes * self){
    struct NodoMensaje * recorreNodos = self->primerNodo;
    while(recorreNodos != NULL){
        printf("Remitente: %s\n"
               "Destinatario: %s\n"
               "Contenido: %s\n", 
               recorreNodos->mensaje->remitente,
               recorreNodos->mensaje->destinatario,
               recorreNodos->mensaje->contenido);
        recorreNodos = recorreNodos->siguiente;
    }
    return;
}

int cantidadMensajes(struct ListaMensajes* self){
    struct NodoMensaje * recorreNodos = self->primerNodo;
    int resultado = 0;
    while(recorreNodos != NULL){
        resultado++;
    }
    return resultado;
}

void limpiarMensajes(struct ListaMensajes* self){
    struct NodoMensaje * recorreNodos = self->primerNodo;
    self->primerNodo = NULL;
    while(recorreNodos != NULL){
        free(recorreNodos->mensaje->contenido);
        free(recorreNodos->mensaje->destinatario);
        free(recorreNodos->mensaje->remitente);
        free(recorreNodos->mensaje);
        struct NodoMensaje * nodoALiberar = recorreNodos;
        free(nodoALiberar);
        recorreNodos = recorreNodos->siguiente;
    }
    return;
}

int cantidadMensajesUsuario(struct ListaMensajes * self, const char * nombreUsuario){
    struct NodoMensaje * recorreNodos = self->primerNodo;
    int resultado = 0;
    while(recorreNodos != NULL){
        if(strcmp(recorreNodos->mensaje->destinatario, nombreUsuario) == 0){
            resultado++;
        }
    }
    return resultado;
}