#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Cliente;
struct NodoContactos;
struct ListaContactos;
struct Mensaje;
struct NodoMensaje;
struct ListaMensajes;
struct Servidor;

struct Servidor{
    char * IpServidor;
    int puertoServidor;
};

struct Cliente{
    char * nombreUsuario;
    int puertoEnUso;
    struct Mensaje * listaMensajes;
    struct  ListaContactos * contactos;
    struct Servidor * servidor;
};

struct NodoContactos{
  char * nombreContacto;
  struct NodoContactos * siguiente;
};

struct ListaContactos{
  struct NodoContactos * primerNodo;
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
void insertarClienteAlInicio(struct ListaContactos *, struct NodoContactos *);
void imprimirListaContactos(struct ListaContactos *);
void limpiarClientes(struct ListaContactos *);
int cantidadClientes(struct ListaContactos *);
int existeCliente(struct ListaContactos *, const char *);

//Funciones de Manipulación de Listas de Mensajes
void insertarMensajeAlInicio(struct ListaMensajes *, struct NodoMensaje *);
void imprimirListaMensajes(struct ListaMensajes *);
void limpiarMensajes(struct ListaMensajes *);
int cantidadMensajes(struct ListaMensajes *);

#endif /* ESTRUCTURAS_H */
