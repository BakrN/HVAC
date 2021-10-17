#include <stdio.h>
#include <stdint.h> 
#include <lib/dplist.h>
#define HASH_TABLE_SIZE 255
// This is a fixed size hash table;  edit to become shared buffer 
typedef struct {
int id 
} element; // the nodes of linked list (sensor data )
typedef struct {
    int sensor_id; 
    double *runningvalues; 
    uint8_t runnig_value_index; 
    double* running_average;  
    dplist_t* list; 
    uint32_t datamgr_index; 
    uint32_t strmgr_index; 
    element* datamgr_element; 
    element* strmgr_element; 
} table_entry; 

typedef enum {
    DATA_ENTRY, STORE_ENTRY
} ENTRY_TYPE; 
typedef struct {
    table_entry* entries; 
    uint16_t count; 
    uint16_t capacity; 
    // capacity is always decided by hash table size 
} hash_table;  


hash_table* create_table(void) ; 
void* add_table_entry(hash_table* map, double value); 
double get_sensor_running_average(hash_table* map, uint16_t sensor_id); 
void destroy_table(hash_table* map);
uint32_t hash_key(uint16_t sensorid);
table_entry* get_entry(ENTRY_TYPE type); 


// Idea is a hash table with circular queues linked lists for each sensor id. then the different threads can work access different sensors. 




