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
void insertarAlInicio(struct ListaClientes * self, struct NodoCliente * nc){
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

int cantidadClientes(ListaClientes* self){
    struct NodoCliente * recorreNodos = self->primerNodo;
    cantidadClientes = 0;
    while(recorreNodos != NULL){
        cantidadClientes++;
    }
    return cantidadClientes;
}

int existeCliente(struct ListaClientes * self, const char * nombreUsuario){
    struct NodoCliente * recorreNodos = self->primerNodo;
    while(recorreNodos != NULL){
        if(strcmp(recorreNodos->cliente->nombreUsuario, nombreUsuario) == 0){
            return 1;
        }
    }
    return 0;
}

//Funciones Lista Mensajes
void insertarAlInicio(struct ListaMensajes * self, struct NodoMensaje* nm){
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

int cantidadMensajes(ListaMensajes* self){
    struct NodoMensaje * recorreNodos = self->primerNodo;
    cantidadMensajes = 0;
    while(recorreNodos != NULL){
        cantidadMensajes++;
    }
    return cantidadMensajes;
}