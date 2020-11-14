/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "message.h"

char * packet_to_string(Message * p){
    char * result = (char *)malloc(strlen(p->source) +  strlen(p->data) + 20);
    
    char * temp = (char *)malloc(strlen(p->source) +  strlen(p->data));
    strcpy(temp, p->source);
    strcat(temp, p->data);
    
    // Header
    sprintf(result , "%d:%d:%s" , p->type , p->size , temp);
    
    return result;
}

Message * string_to_packet(char * str){
    Message * newP  = (Message *)malloc(sizeof(Message));
    char * buf= (char *)malloc(50);
    sscanf(str , "%d:%d:%[^\t]", &newP->type , &newP->size, buf);
    printf("R: %s\n%d\n", buf, strlen(buf) - newP->size);
    strncpy(newP->source , buf, strlen(buf) - newP->size );
    newP->source[strlen(buf) - newP->size] = '\0';
    strcpy(newP->data , buf + strlen(buf) - newP->size );
    
    return newP;
}