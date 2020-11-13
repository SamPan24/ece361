/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   hash_table.h
 * Author: sindhupa
 *
 * Created on November 13, 2020, 1:28 AM
 */

#ifndef HASH_TABLE_H
#define HASH_TABLE_H
struct item {
    char * username;
    int connfd;
};

typedef struct item item;

// linked list for collisions
struct CollisionList {
    item* element; 
    struct CollisionList* next;
};

typedef struct CollisionList CollisionList;


typedef struct HashTable {
    long size;
    long count;
    struct CollisionList ** lists;
} HashTable;


// hash table functions
struct HashTable * hash_table_init(int s);
_Bool insert_item(char * word, int connfd, struct HashTable * table);

void print_table(struct HashTable *wc);
void hash_table_destroy(struct HashTable *wc);


#endif /* HASH_TABLE_H */
