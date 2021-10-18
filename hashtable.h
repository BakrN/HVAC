#include <stdio.h>
#include <stdint.h> 
#include <lib/dplist.h>
#include <config.h> 
#define HASH_TABLE_SIZE 255
// This is a fixed size linked list map;  One for data mgr, one for shared buffer 



typedef struct {
    uint32_t key;
    uint8_t runnig_value_index; 
    double* running_average;
    dplist_t* list;     

}datamgr_table_entry; 


typedef struct {
    uint32_t key; // key  
    dplist_t* list; 
    uint32_t tbr_datamgr; 
    uint32_t tbr_strmgr; 
} sbuffer_table_entry; 


typedef struct {
    void** entries; // a list of void ptr 
    void (*free_entry)(void* entry); 
    int (*add_table_entry)(sensor_data_t* data); // 0 for success , -1 otherwise 
    

    uint16_t count; 
    uint16_t capacity; 
    // capacity is always decided by hash table size 
} hash_table;  

hash_table* create_table() ; 

// Core functions
void add_to_list(hash_table* map, sensor_data_t* data); 
void destroy_table(hash_table* map);
uint32_t hash_key(uint32_t id); // FNV_PRIME
void* get_next(hash_table* map, ENTRY_TYPE type); 
int add_table_entry(hash_table*map, sensor_data_t* data); 
// Idea is a hash table with circular queues linked lists for each sensor id. then the different threads can work access different sensors. 

void* get_entry_by_key(hash_table* map, uint32_t index); 


