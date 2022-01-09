/** 
 * \author Abubakr Nada 
 * Last Name: Nada 
 * First Name: Abubakr 
 * Student Number: r0767316   
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

void sbuffer_listelement_free(void **element);

int sbuffer_add_table_entry(void *entry, void *arg);
void sbuffer_free_entry(void *entry);

//###############################################
typedef struct
{
    int thread_id;
    int tbr_count;
} sbuffer_entry_toberead; // to be read count ;

typedef struct
{

    uint32_t key;   // sensor id
    dplist_t *list; // pointers to sensor_data_t
    sbuffer_entry_toberead **to_be_read;
    int tbr_array_size;
} sbuffer_table_entry;
typedef struct
{

    sbuffer_table_entry *entry;
    int thread_id; // unique to each thread assigned by main program ;

} sbuffer_iterator;

typedef struct
{
    unordered_map *map;
    pthread_mutex_t sbuffer_edit_mutex;
    pthread_cond_t sbuffer_element_added;
    sbuffer_iterator **iterators; // Each reader thread will have an id with indices;
    uint8_t reader_thread_count;
    char terminate_threads;
} sbuffer_t;
/**
 * @brief Returns next unread/valid entry relevant to thread with thread_id id 
 * 
 * @param buffer 
 * @param thread_id 
 * @return sbuffer_table_entry* 
 */
sbuffer_table_entry *get_next(sbuffer_t *buffer, int thread_id);

/**
 * @brief Used by reader threads to notify buffer to keep them updated. 
 * 
 * @param buffer 
 * @param thread_id 
 */
void sbuffer_reader_subscribe(sbuffer_t *buffer, int thread_id);

/**
 * @brief Used by reader threads to notify buffer that they unsubscibed. (not listening anymore)
 * 
 * @param buffer 
 * @param thread_id 
 */
void sbuffer_reader_unsubscribe(sbuffer_t *buffer, int thread_id);

char sbuffer_wait_for_data(sbuffer_t *buffer, int thread_id); //return 0 if there is data // returns 1 if terminate reader threads

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
/**
 * @brief Returns data element count to be read in entry by thread with id thread_i d
 * 
 * @param buffer 
 * @param entry 
 * @param thread_id 
 * @return int 
 */
int sbuffer_get_entry_tbr(sbuffer_t *buffer, sbuffer_table_entry *entry, int thread_id);

/**
 * @brief Used to update shared buffer after reading operations 
 * 
 * @param buffer 
 * @param iter 
 * @param count 
 */
void sbuffer_update_iter(sbuffer_t *buffer, sbuffer_iterator *iter, int count);
/**
 * @brief Used to wakeup any sleeping reader threads listening to buffer 
 * 
 * @param buffer 
 */
void sbuffer_wakeup_readerthreads(sbuffer_t *buffer);
/**
 * @brief Returnes iterator pointer corresponding to thread_id  
 * 
 * @param buffer 
 * @param thread_id 
 * @return sbuffer_iterator* 
 */
sbuffer_iterator *sbuffer_iter(sbuffer_t *buffer, int thread_id);
#endif //_SBUFFER_H_
