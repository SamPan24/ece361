/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <string.h>

#include "message.h"

char * packet_to_string(Message * p){
    char * result = (char *)malloc(strlen(p->source) +  strlen(p->data) + 20);
    
    // Header
    sprintf(result , "%d:%d:%s%s" , p->type , p->size , p->source , p->data);
    
    return result;
}

Message * string_to_packet(char * str){
    Message * newP  = (Message *)malloc(sizeof(packet));
    char * buf= (char *)malloc(50);
    sscanf(str , "%d:%d:%s", &newP->type , &newP->size, buf);
    strncpy(newP->source , buf, strlen(buf) - newP->size );
    newP->source[strlen(buf) - newP->size] = '\0';
    strcpy(newP->data , buf + strlen(buf) - newP->size );
    
    return newP;
}