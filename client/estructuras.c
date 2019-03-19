#include <string.h>
#include "estructuras.h"

//Funciones Lista de Clientes
void insertarClienteAlInicio(struct ListaContactos * self, struct NodoContactos * nc){
    nc->siguiente = self->primerNodo;
    self->primerNodo = nc;
    return;
}

void imprimirListaContactos(struct ListaContactos * self){
    struct NodoContactos * recorreNodos = self->primerNodo;
    while(recorreNodos != NULL){
        printf("Nombre de Usuario: %s\n", recorreNodos->nombreContacto);
        recorreNodos = recorreNodos->siguiente;
    }
    return;
}

void limpiarContactos(struct ListaContactos* self){
    struct NodoContactos * recorreNodos = self->primerNodo;
    self->primerNodo = NULL;
    while(recorreNodos != NULL){
        free(recorreNodos->nombreContacto);
        struct NodoContactos * nodoALiberar = recorreNodos;
        free(nodoALiberar);
        recorreNodos = recorreNodos->siguiente;
    }
    return;
}

int cantidadClientes(struct ListaContactos* self){
    struct NodoContactos * recorreNodos = self->primerNodo;
    int resultado = 0;
    while(recorreNodos != NULL){
        resultado++;
    }
    return resultado;
}

int existeCliente(struct ListaContactos * self, const char * nombreContacto){
    struct NodoContactos * recorreNodos = self->primerNodo;
    while(recorreNodos != NULL){
        if(strcmp(recorreNodos->nombreContacto, nombreContacto) == 0){
            return 1;
        }
    }
    return 0;
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
        resultado+=1;
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
