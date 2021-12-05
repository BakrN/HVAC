/**

 * \author Abubakr Ehab Samir Nada
 */

#ifndef _SBUFFER_H_
#define _SBUFFER_H_
#define _GNU_SOURCE
#include "config.h"
#include "hashtable.h"
#include <pthread.h> 
#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS 0
#define SBUFFER_NO_DATA 1

/*hash table implemenetation*/ 

void sbuffer_element_free(void ** element); 

int sbuffer_add_table_entry(void* map, void* arg); 
void sbuffer_free_entry(void*entry); 

//###############################################


typedef struct{
hash_table* map; 
pthread_mutex_t sbuffer_edit_mutex; 
pthread_cond_t sbuffer_element_added; 
sbuffer_table_entry* strmgr_iterator; // used to get next packet; points to entry of sbuffer
sbuffer_table_entry* datamgr_iterator; // used to get next packet; points to entry of sbuffer
} sbuffer_t; 


sbuffer_table_entry* get_next(sbuffer_t* buffer, ENTRY_TYPE type); 

/**
 * Allocates and initializes a new shared buffer
 * \param buffer a double pointer to the buffer that needs to be initialized
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_init(sbuffer_t **buffer);

/**
 * All allocated resources are freed and cleaned up
 * \param buffer a double pointer to the buffer that needs to be freed
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_free(sbuffer_t **buffer);

/**
 * Removes the first sensor data in 'buffer' (at the 'head') and returns this sensor data as '*data'
 * If 'buffer' is empty, the function doesn't block until new sensor data becomes available but returns SBUFFER_NO_DATA
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to pre-allocated sensor_data_t space, the data will be copied into this structure. No new memory is allocated for 'data' in this function.
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data);

/**
 * Inserts the sensor data in 'data' at the end of 'buffer' (at the 'tail')
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to sensor_data_t data, that will be copied into the buffer
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
*/
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data);

void sbuffer_update_entry(sbuffer_t* buffer, sbuffer_table_entry* entry, ENTRY_TYPE type , int count); // 

#endif  //_SBUFFER_H_
