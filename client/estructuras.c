#include <string.h>
#include "estructuras.h"

//Funciones Lista de Contactos
void insertarContactoAlInicio(struct ListaContactos * self, struct NodoContactos * nc){
    nc->siguiente = self->primerNodo;
    self->primerNodo = nc;
    return;
}

void imprimirListaContactos(struct ListaContactos * self){
    printf("Contactos:\n");
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

int cantidadContactos(struct ListaContactos* self){
    struct NodoContactos * recorreNodos = self->primerNodo;
    int resultado = 0;
    while(recorreNodos != NULL){
        resultado++;
    }
    return resultado;
}

int existeContacto(struct ListaContactos * self, const char * nombreContacto){
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
    printf("Mensajes:\n");
    struct NodoMensaje * recorreNodos = self->primerNodo;
    while(recorreNodos != NULL){
        printf("Remitente: %s\n"
               "Contenido: %s\n"
                "-------------------------\n",
               recorreNodos->mensaje->remitente,
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
        recorreNodos = recorreNodos->siguiente;
    }
    return resultado;
}

void limpiarMensajes(struct ListaMensajes* self){
    struct NodoMensaje * recorreNodos = self->primerNodo;
    self->primerNodo = NULL;
    while(recorreNodos != NULL){
        free(recorreNodos->mensaje->contenido);
        free(recorreNodos->mensaje->remitente);
        free(recorreNodos->mensaje);
        struct NodoMensaje * nodoALiberar = recorreNodos;
        free(nodoALiberar);
        recorreNodos = recorreNodos->siguiente;
    }
    return;
}

/**
 * Esta función saca uno de los mensajes y los saca de la lista
 * @param Una lista de mensajes de la cual se va a sacar el mensaje
 * @return Un nodo mensaje con un mensaje adentro
 */
struct NodoMensaje * pop(struct ListaMensajes * self){
    if(self->primerNodo == NULL){
        return NULL;
    }
    struct NodoMensaje * recorreNodos = self->primerNodo;
    self->primerNodo = recorreNodos->siguiente;
    recorreNodos->siguiente = NULL;
    return recorreNodos;
}
