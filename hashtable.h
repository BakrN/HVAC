#define _GNU_SOURCE 
#include <stdio.h>
#include <stdint.h> 

#include "config.h"
#include <malloc.h>
#include <pthread.h>
#define HASH_TABLE_SIZE 255
// This is a fixed size linked list map;  One for data mgr, one for shared buffer 
// pthread defined in main 





typedef struct {
    void* entries; // a list of void ptr 
    void (*free_entry)(void* entry); 
    int (*add_table_entry)(void* map, sensor_data_t* data); // 0 for success , -1 otherwise 
    void (*initialize_table)(void* map, void*arg); 

    uint16_t count; 
    uint16_t capacity; 
    // capacity is always decided by hash table size 
} hash_table;  

// Core functions
hash_table* create_table(void (*free_entry)(void* entry),
    int (*add_table_entry)(void* map, sensor_data_t* data), // 0 for success , -1 otherwise 
    void (*initialize_table)(void* map, void*arg), void* arg) ; 
void destroy_table(hash_table* map);
uint32_t hash_key(uint32_t id); // FNV_PRIME
int add_entry(hash_table*map, sensor_data_t* data); 
void* get_entry_by_index(hash_table* map, uint32_t index); 
void* get_entry_by_key(hash_table* map, uint32_t key); // this assumes no collision. implement later with collision assumption
// Idea is a hash table with circular queues linked lists for each sensor id. then the different threads can work access different sensors. 




