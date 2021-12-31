/**
 * \author Abubakr Ehab Samir Nada
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include "lib/dplist.h"
#include <time.h>

typedef uint16_t sensor_id_t ; 
typedef double sensor_value_t ; 
typedef time_t sensor_ts_t; 
typedef struct {
    uint16_t id;
    double value;
    long ts;
} sensor_data_t;
typedef enum {
    DATA_ENTRY, STORE_ENTRY
} ENTRY_TYPE; 

typedef struct {
    sensor_data_t* data; 
     
} datamgr_element; 



typedef struct{
    uint32_t key; // sensor id; 

    uint16_t room_id; 
    double current_average; 
    dplist_t* list;     

}datamgr_table_entry; 
typedef struct {
    int thread_id; 
    int tbr_count; 
}sbuffer_entry_toberead; // to be read count ; 
typedef struct {

    uint32_t key; // sensor id  
    dplist_t* list; // pointers to sensor_data_t
    sbuffer_entry_toberead** to_be_read; 
    int tbr_array_size; 
} sbuffer_table_entry; 
typedef struct {
    uint32_t sequence_number; 
    time_t timestamp; 
    char* message; 
} log_msg; 
typedef struct {
    char clear_flag; 
    char* terminate_thread; 
    void* buffer; 
    int reader_thread_id; 
    void* logger; 
}strgmgr_args; 
typedef struct {
    int pipefd; 
    void* fp_sensor_map;
    void* buffer; 
    char* terminate_thread; 
    int reader_thread_id; 
    void* logger; 
}datamgr_args; 

typedef struct {
    int port_number ; 
    int pipefd; 
    void* buffer; 
    void* logger; 
} conn_args; 

#endif /* _CONFIG_H_ */
