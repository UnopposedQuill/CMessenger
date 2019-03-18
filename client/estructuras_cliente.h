#ifndef ESTRUCTURAS_CLIENTE_H
#define ESTRUCTURAS_CLIENTE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Cliente{
    char * nombreUsuario;
    int puenteEnUso;
    Mensaje listaMensajes;
    struct  ListaClientes * contactos;
    Servidor servidor;
};

struct NodoCliente{
  struct Cliente * cliente;
  struct NodoCliente * siguienteCliente;
};

struct ListaClientes{
  struct NodoCliente * primerCliente;
};

struct Mensaje{
  char * remitente;
  char * destinatario;
  char * contenido;
  int estado;
};

struct NodoMensaje{
    struct Mensaje * mensaje;
    struct NodoMensaje * siguiente;
};

struct ListaMensajes{
    struct NodoMensaje * primerNodo;
};

//Funciones de Manipulación de Listas de Clientes
void insertarAlInicio(struct ListaClientes *, struct NodoCliente *);
void imprimirListaClientes(struct ListaClientes *);
int cantidadClientes(struct ListaClientes *);
int existeCliente(struct ListaClientes *);

//Funciones de Manipulación de Listas de Mensajes
void insertarAlInicio(struct ListaMensajes *, struct NodoMensaje *);
void imprimirListaMensajes(struct ListaMensajes *);
int cantidadMensajes(struct ListaClientes *);

#endif /* ESTRUCTURAS_H */
