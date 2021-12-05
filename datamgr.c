 

#include "datamgr.h" 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


/**
 *  This method holds the core functionality of your datamgr. It takes in 2 file pointers to the sensor files and parses them. 
 *  When the method finishes all data should be in the internal pointer list and all log messages should be printed to stderr.
 *  \param fp_sensor_map file pointer to the map file
 *  \param fp_sensor_data file pointer to the binary data file
 */
void *datamgr_init(void* args){

    DATAMGR_DATA* datamgr_data = malloc(sizeof(DATAMGR_DATA));
    sbuffer_t** s_ptr = (sbuffer_t**)(((char*)args) + sizeof(int) + sizeof(FILE*)); 
    FILE** f_ptr = (FILE**)(((char*)args) + sizeof(int) ); 
    int pipefd = *(int*)(args); 
    FILE* fp_sensor_map = *f_ptr; 
    sbuffer_t* buffer = *s_ptr; 
    char** t_ptr = (char**)(((char*)args) + sizeof(int) + sizeof(FILE*)+ sizeof(sbuffer_t*)); 
    datamgr_data->datamgr_table = create_table(datamgr_free_entry, datamgr_add_table_entry, datamgr_initialize_table, fp_sensor_map);
    datamgr_data->pollfd = pipefd; 
    datamgr_data->terminate_reader_thread = *t_ptr; 
    datamgr_parse_sbuffer(datamgr_data, buffer); 
    printf("RETURNED FROM PARSING SBUFFER\n"); 
    datamgr_free(datamgr_data); 
    return NULL; 
}
void datamgr_parse_sbuffer(DATAMGR_DATA* datamgr_data, sbuffer_t* buffer){
    sbuffer_table_entry* entry_ptr = NULL; 
    
    while(!(*(datamgr_data->terminate_reader_thread)) ){ //not terminating threads or there are values still in sbuffer 
    
            entry_ptr = get_next(buffer, DATA_ENTRY) ; 
       
                pthread_mutex_lock(&(buffer->sbuffer_edit_mutex)); 
                while(*(datamgr_data->terminate_reader_thread)==0 && entry_ptr==NULL){
                    printf("datamgr sleepiing\n"); 
                    pthread_cond_wait(&(buffer->sbuffer_element_added), &(buffer->sbuffer_edit_mutex)); 
                    entry_ptr = get_next(buffer, DATA_ENTRY) ; 
                    printf("datamgr woken up \n");
                }
          
            
            if(*(datamgr_data->terminate_reader_thread) && entry_ptr ==NULL){// woken up due to termination
       
            printf("Exited datamgr due to termination wakeuyp\n"); 
            pthread_mutex_unlock(&(buffer->sbuffer_edit_mutex)); 
                return ; 
            } 
     
        dplist_node_t* current = entry_ptr->list->head;
        int count =  entry_ptr->tbr_datamgr; 
         // set iterator to null to have it updated when new insertion is made 
        buffer->datamgr_iterator = NULL; 
        pthread_mutex_unlock(&(buffer->sbuffer_edit_mutex)) ;
        for (int i = 0 ; i < count; i++){
            if(add_entry(datamgr_data->datamgr_table, current->element) ==-1){
                // write to pipe with conn mgr 
                sensor_id_t id= ((sensor_data_t*)current->element)->id; 
                if(write(datamgr_data->pollfd, &id, sizeof(sensor_id_t)) < sizeof(sensor_id_t)){
                    // log failed to communicate with connmgr 
                }
            
            } 
            current = current->next; 
        }
        
    //
    
        sbuffer_update_entry(buffer, entry_ptr, DATA_ENTRY, count); 

    }
    //terminated 
    return ; 
} 

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
        add_entry(datamgr_table, data); 
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
    destroy_table(datamgr_data->datamgr_table); 

    close(datamgr_data->pollfd ); 

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
    datamgr_element* data = *element; 
    free(data->data); 
    free(data); 
    data = NULL ;
}
void* datamgr_element_copy(void * element)
{
    return NULL; 
}
int datamgr_element_compare(void * x, void* y)
{
    return 0; 
}


void datamgr_initialize_table(void* map, void* file){
   // read file // format: 
    // initialize running_avg to -273 

    if(file == NULL){

        return ; 
    }
   FILE* fp_sensor_map = (FILE*) file; 
   
   char* line[10] ; 
   datamgr_table_entry* ptr ; 
    
    // add errror 
   while( fgets( line, 10 , fp_sensor_map) != NULL){
       uint16_t room_id, sensor_id; 
       long* entries = (long*)((hash_table*)map)->entries; 
        
       sscanf(line,"%hu %hu\n", &room_id, &sensor_id); 
       uint32_t index = hash_key(sensor_id); 
       if(entries[index] == 0){
           
            ptr = (datamgr_table_entry*)malloc(sizeof(datamgr_table_entry)); 
            ptr->list = dpl_create(datamgr_element_copy,datamgr_element_free,datamgr_element_compare ); 
            ptr->key = sensor_id; 
            ptr->current_average = 0 ;
            ptr->room_id = room_id; 
            ptr->running_value_index = 0; 
            ptr->running_average = malloc(sizeof(double)* 5); 
            for(int i = 0; i < 5; i++){
                ptr->running_average[i] = -273; 
            }
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
            ptr->current_average = 0 ;
            ptr->room_id = room_id; 
            ptr->running_value_index = 0; 
            ptr->running_average = malloc(sizeof(double)* 5); 
            for(int i = 0; i < 5; i++){
                ptr->running_average[i] = -273; 
            }
            entries[index] = (long)ptr; 
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
    sensor_data_t* data = (sensor_data_t*) args;
    uint32_t index = hash_key(data->id) ; 
    // searching if entry already exists (method of adding an entry is linear)
    if( get_entry_by_index(map, index) != NULL )
    {
        datamgr_table_entry* entry = (datamgr_table_entry*)(get_entry_by_index((hash_table*)map, index)); 
        if(entry->key == data->id){
            if(entry->current_average == 0 && entry->running_value_index == 0){
                ((hash_table*)map)->count++; 
            }

                     // Don't need mutex if only adding to index 0, just make sure you aren't checking prev
            datamgr_element* element = malloc(sizeof(datamgr_element)); 
            element->data =data; 
            entry->running_average[entry->running_value_index] = data->value; 
            entry->running_value_index = (entry->running_value_index + 1) % 5; 
            //-273 means not assigned yet
            int count = 0; 
            while(count != 5){
                if(entry->running_average[count] == -273){
                    break; 
                }
                count++; 
            }
            for (int i = 0; i <= count; i++){
                element->current_average += entry->running_average[i]; 
            }
            element->current_average = element->current_average/(count+1); 
            dpl_insert_at_index(entry->list,element, 0 ,0); // not copied 
            return 0; // success 
        }
        uint32_t search = index+1; 
        while( search!= index){
            // search for it linearly    
            entry = (datamgr_table_entry*)(get_entry_by_index((hash_table*)map, index)); 
            if(entry == NULL){
               // NOt supposed to listen to this sensor id
               // stderr
               #ifdef DEBUG 
               fprintf(stderr, "Sensor with id: %d isin't registered to a room ", entry->key); 
               fflush(stderr); 
               #endif
            }
            // check if sensor id already exists
            else if (entry->key == data->id){
                
            if(entry->current_average == 0 && entry->running_value_index == 0){
                ((hash_table*)map)->count++; 
            }
            datamgr_element* element = malloc(sizeof(datamgr_element)); 
            element->data =data; 
            entry->running_average[entry->running_value_index] = data->value; 
            entry->running_value_index = (entry->running_value_index + 1) % 5; 
            //-273 means not assigned yet
            int count = 0; 
            while(count != 5){
                if(entry->running_average[count] == -273.0){
                    break; 
                }
                count++; 
            }
            for (int i = 0; i <= count; i++){
                element->current_average += entry->running_average[i]; 
            }
            element->current_average = element->current_average/(count+1); 
            if(element->current_average > SET_MAX_TEMP){
                fprintf(stderr, "Sensor %hu Exceeded max temp\n", element->data->id); 
                fflush(stderr); 
            }
            if(element->current_average < SET_MIN_TEMP){
                fprintf(stderr, "Sensor %hu went below min temp\n", element->data->id); 
                fflush(stderr); 
            }
            dpl_insert_at_index(entry->list,element, 0 ,0); // not copied 
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
    dpl_free(&ptr->list, 1); 
    free(ptr->running_average); 
    ptr->list = NULL ;
    ptr->running_average = NULL; 
    free(ptr); 
}
