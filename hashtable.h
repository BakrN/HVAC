/** 
 * \author Abubakr Nada 
 * Last Name: Nada 
 * First Name: Abubakr 
 * Student Number: r0767316   
*/
#pragma once
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdint.h> 

#include "config.h"
#include <malloc.h>

#define DEFAULT_UMAP_SIZE 5
// This is a fixed size unordered map;  One for data mgr, one for shared buffer 


#define UMAP_CALL(x) ({ if (x!= 0 ){ \ 
    DEBUG_PRINTF()\
   } \
   }) 


typedef struct {
    int key ; 
    void* entry; 
} umap_entry; 
typedef struct {
    umap_entry* entries; // a list of umap entry pointers 
    void (*free_entry)(void* entry); 
    int (*add_table_entry)(void* map, void* data); // 0 for success , -1 otherwise 
    void (*initialize_table)(void* map, void*arg); 

    uint16_t count; 
    uint16_t capacity; 
    // capacity is always decided by hash table size 
} unordered_map;  

// Core functions
unordered_map* umap_create(void (*free_entry)(void* entry),
    int (*add_table_entry)(void* map, void* data), // 0 for success , -1 otherwise 
    void (*initialize_table)(void* map, void*arg), void* arg) ; // + size 
void umap_destroy(unordered_map* map);
uint32_t hash_key(uint32_t id, int capacity ); // FNV_PRIME
int umap_add_to_entry(unordered_map*map, void* data, int key ); 
void* umap_get_entry_by_index(unordered_map* map, uint32_t index); 

void* umap_get_entry_by_key(unordered_map* map, int key); 

int umap_add_new(unordered_map* map , void* value , int key);

void umap_expand(unordered_map* map , float factor ); // expand map by  factor 


