/** 
 * \author Abubakr Nada 
 * Last Name: Nada 
 * First Name: Abubakr 
 * Student Number: r0767316   
*/
#include "datamgr.h" 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


void *datamgr_init(void* args){
    datamgr_args* data_args = (datamgr_args*)args; 
    DATAMGR_DATA* datamgr_data = malloc(sizeof(DATAMGR_DATA));
    datamgr_data->log_message = malloc(sizeof(log_msg)); 
    datamgr_data->log_message->timestamp = time(0) ; 
    datamgr_data->log_message->sequence_number= 3 ; 

    datamgr_data->datamgr_table = umap_create(datamgr_free_entry, datamgr_add_table_entry, datamgr_initialize_table, data_args->fp_sensor_map);
    datamgr_data->pipefd = data_args->pipefd; 
    datamgr_data->logger = data_args->logger; 
    datamgr_data->reader_thread_id  = data_args->reader_thread_id; 
    sbuffer_reader_subscribe(data_args->buffer,datamgr_data->reader_thread_id ); 
    datamgr_listen_sbuffer(datamgr_data, data_args->buffer); 
    datamgr_free(datamgr_data); 
    return NULL; 
}

/**
 * @brief This funciton takes in a data manager and a shared buffer as arguments the datamgr listens to the sbuffer
 * 
 * @param datamgr_data DATAMGR_DATA ptr 
 * @param buffer shared buffer ptr 
 */
void datamgr_listen_sbuffer(DATAMGR_DATA* datamgr_data, sbuffer_t* buffer){
    if(!(datamgr_data && buffer)) return ; // assert that datamgr and buffer are not null
    long* ptr = malloc(2 * sizeof(void*)); 
    ptr[0] = (long)datamgr_data; 

    while(!(*(buffer->terminate_threads)) ){ //not terminating threads or there are values still in sbuffer 
        if(sbuffer_wait_for_data(buffer, datamgr_data->reader_thread_id)){
            // terminate 
            #ifdef DEBUG
            printf("DATAMGR_TERMINATED\n"); 
            #endif
            free(ptr); 
            return ; 
        }; 

        sbuffer_iterator* iter = sbuffer_iter(buffer, datamgr_data->reader_thread_id); 
        
        dplist_node_t* current = iter->entry->list->head;

        int count = sbuffer_get_entry_tbr(buffer, iter->entry, datamgr_data->reader_thread_id); // error if -1 
        if(count ==-1 ){// 
        #ifdef DEBUG
        printf("ERROR COULDN't find tbr of datamgr reader thread\n") ; 
        #endif
        continue; 
        }

        for (int i = 0 ; i < count ; i ++){
            ptr[1] = (long) current->element; 
            if(umap_addentry(datamgr_data->datamgr_table,ptr) ==-1){  // insert copy of data into datamgr 
                // write to pipe with conn mgr 
                sensor_id_t id= ((sensor_data_t*)current->element)->id; 
                if(write(datamgr_data->pipefd, &id, sizeof(sensor_id_t)) < sizeof(sensor_id_t)){
                    // log failed to communicate with connmgr 
                }
            
            } 
            current = current->next; 
        

        }
        
    //
    
        sbuffer_update_iter(buffer, iter, count); 

    }
    //terminated 
    free(ptr); 
    return ; 
} 



/**
 *  This method holds the core functionality of your datamgr. It takes in 2 file pointers to the sensor files and parses them. 
 *  When the method finishes all data should be in the internal pointer list and all log messages should be printed to stderr.
 *  \param fp_sensor_map file pointer to the map file
 *  \param fp_sensor_data file pointer to the binary data file
 */
/*void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data){
    // check if files are not null 
    // create table if not createdd 
    datamgr_table = 
    // binary file read 
  
    char* buffer = malloc(18); 
    while (fread(buffer, 18,1, fp_sensor_data) == 1){  
        sensor_data_t* data = malloc(sizeof(sensor_data_t));  
        data->id = *(uint16_t*)buffer; 
        buffer += 2; 
        data->value = *(double*)buffer; 
        buffer += 8 ; 
        data->ts = *(time_t*)buffer;  
        umap_addentry(datamgr_table, data); 
        buffer -=10; 
    }
  
    free(buffer) ;
    buffer = 0; 

}
*/

/**
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free(DATAMGR_DATA* datamgr_data){
    umap_destroy(datamgr_data->datamgr_table); 
    
    close(datamgr_data->pipefd ); 
    free(datamgr_data->log_message); 
    free(datamgr_data); 
    
}

/**
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the corresponding room id
 */
uint16_t datamgr_get_room_id(DATAMGR_DATA* datamgr_data,sensor_id_t sensor_id){
    datamgr_table_entry* ptr = (datamgr_table_entry*)get_entry_by_key(datamgr_data->datamgr_table, sensor_id); 
    ERROR_HANDLER(ptr == NULL && ptr->key != sensor_id,  "invalid sensorid ") ; 
    return ptr->room_id; 
}

/**
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the running AVG of the given sensor
 */
sensor_value_t datamgr_get_avg(DATAMGR_DATA* datamgr_data,sensor_id_t sensor_id){
    datamgr_table_entry* ptr = (datamgr_table_entry*)get_entry_by_key(datamgr_data->datamgr_table, sensor_id); 
    ERROR_HANDLER(ptr == NULL && ptr->key != sensor_id,  "invalid sensorid ") ; 
    return ptr->current_average; 
}

/**
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the last modified timestamp for the given sensor
 */
time_t datamgr_get_last_modified(DATAMGR_DATA* datamgr_data,sensor_id_t sensor_id){
     datamgr_table_entry* ptr = (datamgr_table_entry*)get_entry_by_key(datamgr_data->datamgr_table, sensor_id); 
    ERROR_HANDLER(ptr == NULL && ptr->key != sensor_id,  "invalid sensorid ") ; 
    return ((sensor_data_t*)dpl_get_element_at_index(ptr->list, 0))->ts; 
}

/**
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 *  \return the total amount of sensors
 */
int datamgr_get_total_sensors(DATAMGR_DATA* datamgr_data){
    return datamgr_data->datamgr_table->count; // this is the number of unique sensor id by sensor map; 
}


/**
 * Hash table implementations and dp list related functions 
 * */ 
void datamgr_element_free(void ** element)
{
    sensor_data_t* data = *element; 
    free(data); 

    *element = NULL ;
}
void* datamgr_element_copy(void * element)
{
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    data->id = ((sensor_data_t*)element)->id; 
    data->ts = ((sensor_data_t*)element)->ts; 
    data->value = ((sensor_data_t*)element)->value; 
    return data; 
}
int datamgr_element_compare(void * x, void* y)
{

    return 0; 
}


void datamgr_initialize_table(void* map, void* file){

    // this function intializes the data structure that the datamgr uses 

    if(file == NULL){

        return ; 
    }
   FILE* fp_sensor_map = (FILE*) file; 
   
   char line[20] ; 
   datamgr_table_entry* ptr ; 
    
    // add errror 
   while( fgets( line, 10 , fp_sensor_map) != NULL){
       uint16_t room_id, sensor_id; 
       long* entries = (long*)((hash_table*)map)->entries; 
        
       sscanf(line,"%hu %hu", &room_id, &sensor_id); 
       uint32_t index = hash_key(sensor_id); 
       if(entries[index] == 0){
           
            ptr = (datamgr_table_entry*)malloc(sizeof(datamgr_table_entry)); 
            ptr->list = dpl_create(datamgr_element_copy,datamgr_element_free,datamgr_element_compare ); 
            ptr->key = sensor_id; 
            ptr->current_average = -500.0 ; // initial value
            ptr->room_id = room_id; 
            ptr->list_size = 0 ; 
            entries[index] = (long)ptr; 
            continue; 
       }
       uint32_t count = index; 
      
       while(entries[count] != 0){
           
           count++; 
           if(count >= ((hash_table*)map)->capacity){
               count = 0; 
           }
           if(count == index) {
               #ifdef DEBUG
                printf("HASHTABLE FULL"); 
               #endif
                printf("HASHTABLE FULL");
               return; 
           }
       }
         
            ptr = (datamgr_table_entry*)malloc(sizeof(datamgr_table_entry)); 
            ptr->list = dpl_create(datamgr_element_copy,datamgr_element_free,datamgr_element_compare ); 
            ptr->key = sensor_id; 
            ptr->current_average = -500.0 ; // initial value is -500
            ptr->room_id = room_id; 
            ptr->list_size = 0 ; 
      
            entries[count] = (long)ptr; 
   }


    if(fclose(fp_sensor_map) != 0){
        #ifdef DEBUG
        fprintf(stderr, "File wasn't closed. "); 
        fflush(stderr); 
        #endif
        return ; 
    }
    
}

//for testing
/*dplist_t* datamgr_get_list_by_key(uint32_t key){
    uint32_t index = hash_key(key); 
    long* entries = (long*)datamgr_table->entries;
    return ((datamgr_table_entry*)entries[index])->list; 
}*/

int datamgr_add_table_entry(void* map, void* args){
    // No mutex needed because no
    // steps: check if key exists if not create it 
    long* ptr = (long* )args; 
    sensor_data_t* data = (sensor_data_t*) ptr[1];
    DATAMGR_DATA* datamgr_data = (DATAMGR_DATA*) ptr[0]; 
    uint32_t index = hash_key(data->id) ; 
    // searching if entry already exists (method of adding an entry is linear)
    if( umap_get_entry_by_index(map, index) != NULL )
    {
        datamgr_table_entry* entry = (datamgr_table_entry*)(umap_get_entry_by_index((hash_table*)map, index)); 
        if(entry->key == data->id){
            if(entry->list_size ==0 ){
                ((hash_table*)map)->count++; 
                entry->current_average = 0; 
            }

                     // Don't need mutex if only adding to index 0, just make sure you aren't checking prev
            
            if(entry->list_size == 5){
                dpl_remove_at_index(entry->list, 4, 1); 
                entry->list_size--; 
            }
            dpl_insert_at_index(entry->list,data, 0 ,1); // copied 
            entry->list_size++; 
            entry->current_average += (data->value - entry->current_average)/entry->list_size; 
            if(entry->current_average > SET_MAX_TEMP){
                datamgr_data->log_message->timestamp = time(0);
                asprintf(&(datamgr_data->log_message->message), "The sensor node wtih id: %hu reports it's too hot (running avg temperature = %f)", data->id, entry->current_average);
                log_event( datamgr_data->log_message, datamgr_data->logger);
                free(datamgr_data->log_message->message);
                #ifdef DEBUG
                fprintf(stderr, "Sensor %hu Exceeded max temp\n", data->id); 
                fflush(stderr); 
                #endif
            }
            if(entry->current_average < SET_MIN_TEMP){
                    datamgr_data->log_message->timestamp = time(0);
                asprintf(&(datamgr_data->log_message->message), "The sensor node wtih id: %hu reports it's too cold (running avg temperature = %f)", data->id, entry->current_average);
                log_event( datamgr_data->log_message, datamgr_data->logger);
                free(datamgr_data->log_message->message);
                #ifdef DEBUG
                fprintf(stderr, "Sensor %hu went below min temp\n", data->id); 
                fflush(stderr); 
                #endif
            }
            
            return 0; // success 
        }
        uint32_t search = index+1; 
        while( search!= index){
            // search for it linearly    
            entry = (datamgr_table_entry*)(umap_get_entry_by_index((hash_table*)map, index)); 
            if(entry == NULL){
               // NOt supposed to listen to this sensor id
               // stderr
               #ifdef DEBUG 
               fprintf(stderr, "Sensor with id: %d isn't registered to a room YE\n", data->id); 
               fflush(stderr); 
               #endif
            }
            // check if sensor id already exists
            else if (entry->key == data->id){
                
            if(entry->list_size == 0 ){
                ((hash_table*)map)->count++; 
                entry->current_average = 0; 
            }
            if(entry->list_size == 5){
                entry->list = dpl_remove_at_index(entry->list, 4, 1); 
                entry->list_size--; 
            }
            entry->list = dpl_insert_at_index(entry->list,data, 0 ,1); // copied 
            entry->list_size++; 
            entry->current_average += (entry->current_average)/entry->list_size; 
            if(entry->current_average > SET_MAX_TEMP){
                   datamgr_data->log_message->timestamp = time(0);
                asprintf(&(datamgr_data->log_message->message), "The sensor node wtih id: %hu reports it's too hot (running avg temperature = %f)", data->id, entry->current_average);
                log_event( datamgr_data->log_message, datamgr_data->logger);
                free(datamgr_data->log_message->message);
                #ifdef DEBUG
                fprintf(stderr, "Sensor %hu Exceeded max temp\n", data->id); 
                fflush(stderr); 
                #endif 
            }
            if(entry->current_average < SET_MIN_TEMP){
                     datamgr_data->log_message->timestamp = time(0);
                asprintf(&(datamgr_data->log_message->message), "The sensor node wtih id: %hu reports it's too cold (running avg temperature = %f)", data->id, entry->current_average);
                log_event( datamgr_data->log_message, datamgr_data->logger);
                free(datamgr_data->log_message->message);
                #ifdef DEBUG
                fprintf(stderr, "Sensor %hu went below min temp\n", data->id); 
                fflush(stderr); 
                #endif
            }
            return 0; // success 
        
          
            }

            if(search++ == HASH_TABLE_SIZE){
                search = 0;// wrap back around 
            }
        }
    
        // not found
        // this shouldn't happen
        return -1; 
    }
    // Not listening to this sensor
    return -1; 
}
void datamgr_free_entry(void*entry){
    datamgr_table_entry*  ptr = (datamgr_table_entry*)entry; 
    dpl_free(&(ptr->list), 1); 

    ptr->list = NULL ;
    
    
    free(ptr); 
    entry=NULL; 
}

/** 
 *FIXED  HASH TABLE IMPLEMENTATION
 * 
 * */

hash_table* umap_create(void (*free_entry)(void* entry),
    int (*add_table_entry)(void* map, void* data), // 0 for success , -1 otherwise 
    void (*initialize_table)(void* map, void*arg), void* arg){

    hash_table* mp = malloc(sizeof(hash_table));
    
    

    mp->entries = calloc(HASH_TABLE_SIZE, sizeof(void*)); 
    long* entries = (long*)mp->entries; 
    

    for( int i = 0; i < HASH_TABLE_SIZE; i++){
        
        
        entries[i] = (long)NULL; 
   
    }

    
    mp->add_table_entry = add_table_entry;  
    mp->free_entry = free_entry; 

    mp->capacity = HASH_TABLE_SIZE; 
    mp->count = 0; 
    

    
    
    
    if(initialize_table != NULL){
         mp->initialize_table = initialize_table; 
        initialize_table((void*)mp,arg); 
    }
 

    return mp; 
}
void* umap_get_entry_by_index(hash_table* map, uint32_t index){

    long* entries =  (long*)map->entries; 
    return (void*)entries[index]; 
}
void umap_destroy(hash_table* map){

    long* entries =  (long*) map->entries; 
    for(int i = 0; i < map->capacity; i++){
        
       if(entries[i] != (long)NULL && map->free_entry != NULL){
         
           map->free_entry((void*)entries[i]);
            entries[i] = (long)NULL; 
        } 
   }

    free(map->entries); 
    free(map); 
    map = NULL; 
}      
void* get_entry_by_key(hash_table* map, uint32_t key){
    long* ptr = (long*) map->entries; 
    return (void*)ptr[hash_key(key)] ; 
}
int umap_addentry(hash_table*map, void* data){
    
    return map->add_table_entry((void*)map, data) ; 
}
uint32_t hash_key(uint32_t id){
    uint32_t hash = 2166136261; //32 bit offset
    uint32_t FNV_prime = 16777619 ;
    return ((hash*FNV_prime)^ id) % HASH_TABLE_SIZE;
    /*
   hash = offset_basis
    for each octet_of_data to be hashed
     hash = hash * FNV_prime
     hash = hash xor octet_of_data
    return hash

32 bit offset_basis = 2166136261
32 bit FNV_prime = 224 + 28 + 0x93 = 16777619*/ 
}


void umap_expand(hash_table* map , float factor ){
    if(factor <= 1 ){
        return ; 
    }
    int new_size = (int) ( map->capacity * factor)  ;
    if (map->capacity == new_size){
        new_size++ ; 
    }
    map->entries = realloc(map->entries, new_size* sizeof(long)); 
    // recalculate indices 
    
    // re
}