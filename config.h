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
    double current_average; 
} datamgr_element; 



typedef struct{
    uint32_t key; // sensor id; 

    uint16_t room_id; 
    uint8_t running_value_index; 
    double* running_average;
    double current_average; 
    dplist_t* list;     

}datamgr_table_entry; 


typedef struct {
    uint32_t key; // sensor id  
    dplist_t* list; // pointers to sensor_data_t
    uint32_t tbr_datamgr; 
    uint32_t tbr_strmgr; 
} sbuffer_table_entry; 
typedef struct {
    uint32_t sequence_number; 
    time_t timestamp; 
    char* message; 
} log_msg; 

#endif /* _CONFIG_H_ */
