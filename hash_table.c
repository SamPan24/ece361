/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include<string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hash_table.h"

// linked list functions
void free_list(CollisionList* list);
CollisionList* create_list_item ();
CollisionList* insert_list(CollisionList* list, item* element);
void print_list(CollisionList* list);

// Hash key Function
long hash( char *str);

// https://stackoverflow.com/questions/7666509/hash-function-for-string
long hash( char *str)
{
    long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

struct HashTable * hash_table_init(int s){
    struct HashTable * table;

    table = (struct HashTable *)malloc(sizeof(struct HashTable));
    
    table->size = s ; // change to another number
    table->count = 0;
    
    table->lists = (struct CollisionList **)malloc(table->size * sizeof(struct CollisionList *));
    for(int i = 0 ; i < table->size; i++){
        table->lists[i] = NULL;
    }
    
    return table;
}

_Bool insert_item(char * word, int connfd, struct HashTable * table){

    // create an item
    item * element = (item *)malloc(sizeof(item));
    element->username = (char *)malloc( (strlen(word) + 1) * sizeof(char));
    strcpy(element->username , word );
    element->connfd = connfd;
    
    // insert
    int index = hash(word) ;
    index = (index & 0x7fffffff) % table->size;
    
    
    CollisionList * p = insert_list(table->lists[index] , element);
    if(p != NULL)
        table->lists[index] = p;
    else{
        return false;
    }
    table->count++;
    return true;
}

void print_table(struct HashTable *wc)
{
    for(int i = 0 ; i < wc->size; i++){
        if(wc->lists[i] != NULL){
            print_list(wc->lists[i]);
        }
            
    }
}

void hash_table_destroy(struct HashTable *wc)
{
    for(int i = 0 ; i < wc->size; i++){
        if(wc->lists[i] != NULL){
            free_list(wc->lists[i]);
            wc->lists[i] = NULL;
        }
    }
    if(wc->lists != NULL)
        free(wc->lists);
    free(wc);
}

void print_list(CollisionList* list){
    CollisionList * temp = list;
    while(temp != NULL){
        printf("%s:%d\n" , temp->element->username , temp->element->connfd);
        temp = temp->next;
    }
}
 
CollisionList* create_list_item () {
    // Allocates memory for a LinkedCollisionList pointer
    CollisionList* list = (CollisionList*) malloc (sizeof(CollisionList));
    return list;
}

CollisionList* insert_list(CollisionList* list, item* element) {
    // Check if list is empty
    if (!list) {
        CollisionList* head = create_list_item();
        head->element = element;
        head->next = NULL;
        list = head;
        return list;
    } 
     
    
    CollisionList* prev = list;
    CollisionList* temp = list->next;
    
    do{
        // check if item if found already
        if( strcmp(prev->element->username , element->username) == 0 ){
            // found
            // Same word exists, no collision
            // Same user already logged in
            
            free(element);
            return NULL;
        }
        
        // if not traverse through the list
        if(temp != NULL){
            prev = temp;
            temp = temp->next;
        }
        else{
            break;
        }
    }while(1);
    
    // temp points to last element in the list, element was not found
    // new item
    
    
    
    CollisionList* node = create_list_item();
    node->element = element;
    node->next = NULL;
    prev->next = node;
    
    //printf("Collision! Elements (list):\n");
    //print_list(list);
    
    return list;
}

void free_list(CollisionList* list) {
    CollisionList* prev = list;
    if(list == NULL){
        return;
    }
    while (list) {
        prev = list;
        list = list->next;
        free(prev->element->username);
        free(prev->element);
        free(prev);
    }
}


