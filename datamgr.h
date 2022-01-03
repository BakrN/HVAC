/** 
 * \author Abubakr Nada 
 * Last Name: Nada 
 * First Name: Abubakr 
 * Student Number: r0767316   
*/
#ifndef DATAMGR_H_
#define DATAMGR_H_


#include "sbuffer.h"
#include "config.h"
#include "hashtable.h"
#include "logger.h"
#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP
#define SET_MAX_TEMP 40
//#error SET_MAX_TEMP not set
#endif

#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP 5
//#error SET_MIN_TEMP not set
#endif

/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#define ERROR_HANDLER(condition, ...)    do {                       \
                      if (condition) {                              \
                        printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
                        exit(EXIT_FAILURE);                         \
                      }                                             \
                    } while(0)

typedef struct{
    uint32_t key; // sensor id; 

    uint16_t room_id; 
    double current_average; 
    dplist_t* list;     
    int list_size ; 
 
}datamgr_table_entry; 
typedef struct{
  
    int pipefd; 

    int reader_thread_id; 
    hash_table* datamgr_table; 


    void* retval ;
    logger_t* logger; 
    log_msg* log_message; 
} DATAMGR_DATA; 



/**
 *  This method holds the core functionality of your datamgr. It takes in 2 file pointers to the sensor files and parses them. 
 *  When the method finishes all data should be in the internal pointer list and all log messages should be printed to stderr.
 *  \param fp_sensor_map file pointer to the map file
 *  \param fp_sensor_data file pointer to the binary data file
 */
void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data);

/**
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free(DATAMGR_DATA* datamgr_data);

/**
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the corresponding room id
 */
uint16_t datamgr_get_room_id(DATAMGR_DATA* datamgr_data , sensor_id_t sensor_id);

/**
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the running AVG of the given sensor
 */
sensor_value_t datamgr_get_avg(DATAMGR_DATA* datamgr_data,  sensor_id_t sensor_id);

/**
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the last modified timestamp for the given sensor
 */
time_t datamgr_get_last_modified(DATAMGR_DATA* datamgr_data, sensor_id_t sensor_id);

/**
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 *  \return the total amount of sensors
 */
int datamgr_get_total_sensors(DATAMGR_DATA* datamgr_data );

/**
 * Hash table implementations and dp list related functions 
 * */ 
void datamgr_element_free(void ** element); 
void* datamgr_element_copy(void * element); 
int datamgr_element_compare(void * x, void* y); 

void datamgr_initialize_table(void* map, void* file); 
int datamgr_add_table_entry(void* map, void* args); 
void datamgr_free_entry(void*entry); 

void* datamgr_init(void* args); 
void datamgr_parse_sbuffer(DATAMGR_DATA* datamgr_data, sbuffer_t* buffer); 
//* TESTING FUNCS TO BE REMOVED LATER*/ 
dplist_t* datamgr_get_list_by_key(uint32_t key); 

#endif  //DATAMGR_H_
