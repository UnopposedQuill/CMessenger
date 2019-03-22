/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   estructuras.h
 * Author: esteban
 *
 * Created on 18 de marzo de 2019, 12:32 AM
 */

#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdlib.h>
#include <stdio.h>

struct Cliente;
struct NodoCliente;
struct ListaClientes;
struct Mensaje;
struct NodoMensaje;
struct ListaMensajes;

struct Cliente{
    char * nombreUsuario;
    char * ipRegistrada;
    int puertoRegistrado;
    struct ListaClientes * contactos;
};

struct NodoCliente{
    struct Cliente * cliente;
    struct NodoCliente * siguiente;
};

struct ListaClientes{
    struct NodoCliente * primerNodo;
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
void insertarClienteAlInicio(struct ListaClientes *, struct NodoCliente *);
void imprimirListaClientes(struct ListaClientes *);
void limpiarClientes(struct ListaClientes *);
int cantidadClientes(struct ListaClientes *);
int existeCliente(struct ListaClientes *, const char *);
struct Cliente * buscar(struct ListaClientes *, const char *);

//Funciones de Manipulación de Listas de Mensajes
void insertarMensajeAlInicio(struct ListaMensajes *, struct NodoMensaje *);
void imprimirListaMensajes(struct ListaMensajes *);
void limpiarMensajes(struct ListaMensajes *);
int cantidadMensajes(struct ListaMensajes *);
int cantidadMensajesUsuario(struct ListaMensajes *, const char *);

#endif /* ESTRUCTURAS_H */
