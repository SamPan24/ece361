/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   message.h
 * Author: sindhupa
 *
 * Created on November 12, 2020, 10:29 PM
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_NAME 50
#define MAX_DATA 1024


typedef enum MessageType {
    LOGIN ,
    LO_ACK ,
    LO_NAK ,
    EXIT ,
    JOIN ,
    JN_ACK ,
    JN_NAK,
    LEAVE_SESS ,
    NEW_SESS ,
    NS_ACK ,
    MESSAGE ,
    QUERY ,
    QU_ACK
} MessageType;

typedef struct Message {
    MessageType type;
    unsigned int size;
    char source[MAX_NAME];
    char data[MAX_DATA];
    
} Message;


char * packet_to_string(Message * p);
Message * string_to_packet(char * str);

#endif /* MESSAGE_H */

