#define _GNU_SOURCE 
#include <stdio.h>
#include <stdint.h> 
#include <lib/dplist.h>
#include <config.h> 
#include <malloc.h>
#include <pthread.h>
#define HASH_TABLE_SIZE 255
// This is a fixed size linked list map;  One for data mgr, one for shared buffer 
// pthread defined in main 

extern sbuffer_table_entry* strmgr_iterator; // used to get next packet; points to entry of sbuffer
extern sbuffer_table_entry* datamgr_iterator; // used to get next packet; points to entry of sbuffer




typedef struct {
    void** entries; // a list of void ptr 
    void (*free_entry)(void* entry); 
    int (*add_table_entry)(sensor_data_t* data); // 0 for success , -1 otherwise 
    void (*initialize_table)(hash_table* map, void*arg); 

    uint16_t count; 
    uint16_t capacity; 
    // capacity is always decided by hash table size 
} hash_table;  

// Sbuffer funcs
void sbuffer_element_free(void ** element); 

int sbuffer_add_table_entry(hash_table* map, sensor_data_t* data); 
void sbuffer_free_entry(void*entry); 
//sbuffer end


// Datamgr funcs
void datamgr_element_free(void ** element); 
void* datamgr_element_copy(void * element); 
int datamgr_element_compare(void * x, void* y); 

void datamgr_initialize_table(hash_table* map, void* file); 
int datamgr_add_table_entry(hash_table* map, sensor_data_t* data); 
void datamgr_free_entry(void*entry); 
//end

// Core functions
hash_table* create_table(void (*free_entry)(void* entry),
    int (*add_table_entry)(sensor_data_t* data), // 0 for success , -1 otherwise 
    void (*initialize_table)(hash_table* map, void*arg), void* arg) ; 
void destroy_table(hash_table* map);
uint32_t hash_key(uint32_t id); // FNV_PRIME
sbuffer_table_entry* get_next(hash_table* map, ENTRY_TYPE type); 
int add_table_entry(hash_table*map, sensor_data_t* data); 
// Idea is a hash table with circular queues linked lists for each sensor id. then the different threads can work access different sensors. 




