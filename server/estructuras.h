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

struct NodoCliente;
struct Cliente;
struct ListaClientes;
struct Mensaje;

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
    
    //void (*insertarAlInicio)(struct ListaClientes *, struct NodoCliente *);
};

void insertarAlInicio(struct ListaClientes *, struct NodoCliente *);
void imprimirListaClientes(struct ListaClientes *);

struct Mensaje{
    const char * remitente;
    const char * destinatario;
    const char * contenido;
    int estado;
};

#endif /* ESTRUCTURAS_H */
