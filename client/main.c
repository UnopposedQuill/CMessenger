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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define True 1
#define False 0

/*
 * 
 */
int main(int argc, char** argv) {

    //Realizo un fork, y guardo el retorno en pid
    int pid = fork();
    //Si es menor a cero, no pudo realizarlo, error
    if(pid < 0){
        return (EXIT_FAILURE);
    }
    //Si es cero, entonces es el subproceso, es el que va a estar recibiendo los nuevos mensajes
    else if(pid = 0){
        
    }
    //Es mayor a cero, es el proceso pariente, que va a desplegar el menú y funciones
    else{
        
    }
    return (EXIT_SUCCESS);
}

