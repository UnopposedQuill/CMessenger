/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   utils.c
 * Author: esteban
 * 
 * Created on 20 de marzo de 2019, 03:16 PM
 */

#include "utils.h"

int cantidadDigitos(int numero){
    if(numero == 0){
        return 1;
    }
    int contador = 0;
    while(numero != 0){
        numero /= 10;
        contador++;
    }
    return contador;
}

char * strncat2(char * s, const char * ct, int n){
    char * t = s;
    while(*(t++));
    while((*(t++)=*(ct++)) && (n-- > 0));
    t--;
    do{
        *(t++)='\0';
    }while(n-- > 0);
    return s;
}
