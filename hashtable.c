#include "hashtable.h" 


hash_table* create_table(void (*free_entry)(void* entry),
    int (*add_table_entry)(void* map, sensor_data_t* data), // 0 for success , -1 otherwise 
    void (*initialize_table)(void* map, void*arg), void* arg){

    hash_table* mp = malloc(sizeof(hash_table));
    
    
    mp->entries =malloc(HASH_TABLE_SIZE* sizeof(void*)); 
    
    char**entries = &mp->entries; 
    

    for( int i = 0; i < HASH_TABLE_SIZE; i++){
        // entries[i] = mp->initialize_entry((void*)(entries[i])); 
        entries[i] = NULL; 
   
    }
      for( int i = 0; i < HASH_TABLE_SIZE; i++){
        // entries[i] = mp->initialize_entry((void*)(entries[i])); 
      
   
    }
    
    mp->add_table_entry = add_table_entry;  
    mp->free_entry = free_entry; 

    mp->capacity = HASH_TABLE_SIZE; 
    mp->count = 0; 
    

    
    
    
    if(initialize_table != NULL){
         mp->initialize_table = initialize_table; 
        (*mp->initialize_table)((void*)mp,arg); 
    }
 

    return mp; 
}
void* get_entry_by_index(hash_table* map, uint32_t index){

    char** entries =  &map->entries; 
    return entries[index]; 
}
void destroy_table(hash_table* map){

    char** entries =  &map->entries; 
    for(int i = 2; i < map->capacity; i++){
       if(entries[i] != NULL && map->free_entry != NULL){
           printf("At index: %d , %d\n", i, (int)entries[i]); 
           (*map->free_entry)((void*)entries[i]);
            entries[i] = NULL; 
        } 
   }

    free(map->entries); 
    free(map); 
    map = NULL; 
}      
int add_entry(hash_table*map, sensor_data_t* data){
  
    return (*map->add_table_entry)((void*)map, data) ; 
}
//sbuffer function implementations
int sbuffer_add_table_entry(void* map, sensor_data_t* data){
    
    // steps: check if key exists if not create it /*
    uint32_t index = hash_key(data->id) ; 
    // searching if entry already exists (method of adding an entry is linear)
    if( get_entry_by_index(map, index) != NULL )
    {
        sbuffer_table_entry* entry = (sbuffer_table_entry*)(get_entry_by_index((hash_table*)map, index)); 
        if(entry->key == data->id){

            // write lock/ mutex
            if (entry->tbr_strmgr == 0 && entry->tbr_datamgr == 0){
                // all elements read; 
                // free list
                          
            }

            // Don't need mutex if only adding to index 0, just make sure you aren't checking prev
        
            dpl_insert_at_index(entry->list,data, 0 ,0); // not copied 
            
        
            entry->tbr_datamgr +=1; 
            entry->tbr_strmgr += 1; 
            //unlock
            return 0; // success 
        }
        uint32_t search = index+1; 
        while( search!= index){
            // search for it linearly    
            entry = (sbuffer_table_entry*)(get_entry_by_index(map, index)); 
            if(entry == NULL){
                // create 
                entry = malloc(sizeof(sbuffer_table_entry)); 
                entry->key = data->id; 
                entry->list = dpl_create(NULL,sbuffer_element_free, NULL); 
                dpl_insert_at_index(entry->list, data, 0, 0); 
                // no mutex needed because no entry can acess; it 
                entry->tbr_datamgr = 1; 
                entry->tbr_strmgr = 1;
         
                 //unlock
                  
                char** entries = &((hash_table*)map)->entries;  
                entries[index] = (void*)entry; 
                return 0;  
            }
            // check if sensor id already exists
            else if (entry->key == data->id){
                            // write lock/ mutex
            if (entry->tbr_strmgr == 0 && entry->tbr_datamgr == 0){
                // all elements read; 
                // free list
                          
            }

            // Don't need mutex if only adding to index 0, just make sure you aren't checking prev
            dpl_insert_at_index(entry->list,data, 0 ,0); // not copied 
            
        
            entry->tbr_datamgr +=1; 
            entry->tbr_strmgr += 1; 
            //unlock
            return 0; // success 
            }

            if(search++ == HASH_TABLE_SIZE){
                search = 0;// wrap back down 
            }
        }
    
        // not found
        // this shouldn't happen
        return -1; 
    }
  
  
        // create new entry
    sbuffer_table_entry* new_entry = malloc(sizeof(sbuffer_table_entry));  
    new_entry->key = data->id; 
    new_entry->list = dpl_create(NULL,sbuffer_element_free, NULL); 
    dpl_insert_at_index(new_entry->list, data, 0, 0); 
    // no mutex needed because no entry can acess; it 
    new_entry->tbr_datamgr = 1; 
    new_entry->tbr_strmgr = 1;
     
    //unlock
    char** entries = &((hash_table*)map)->entries; 
    entries[index] = (void*)new_entry; 
    return 0; 
 
}
void sbuffer_free_entry(void*entry){
     sbuffer_table_entry* ptr = (sbuffer_table_entry*)entry; 
    printf("Entered free entry even though I shouldn't\n", entry); 
    //dpl_free(ptr->list, 1); 
    //ptr->list = NULL ;
    //free(ptr); 
    //ptr = NULL; 
}
void sbuffer_element_free(void ** element){
    free(*element); // frees sensor_data_id pointer; 
    *element=  NULL; 
}

sbuffer_table_entry* get_next(hash_table* map, ENTRY_TYPE type){
    //pthread_rwlock_rdlock(&sbuffer_edit_mutex); 
   
    char** entries =  &map->entries;  
    sbuffer_table_entry* entry; 
    for(int i =0; i < HASH_TABLE_SIZE; i++){
        entry = (sbuffer_table_entry*) entries[i];      
        if(type == DATA_ENTRY){
            if(entry != NULL && datamgr_iterator != entry && entry->tbr_datamgr> 0){
                datamgr_iterator= entry; 
                //pthread_rwlock_unlock(&sbuffer_edit_mutex); 
                return entry;
            }
        }
        else{
            if(entry != NULL && strmgr_iterator != entry && entry->tbr_strmgr> 0){
                strmgr_iterator = entry; 
                //pthread_rwlock_unlock(&sbuffer_edit_mutex); 
                return entry;
            }
        }
    }
    //pthread_rwlock_unlock(&sbuffer_edit_mutex); 
    return NULL;     
}
//sbuffer end 

//datamgr implemnetations 
void datamgr_initialize_table(void* map, void* file){
   // read file // format: 
    // initialize running_avg to -273 

    if(file == NULL){

        return ; 
    }
   FILE* fp_sensor_map = (FILE*) file; 
   
   char* line; 
   datamgr_table_entry* ptr ; 
   fopen(fp_sensor_map, 'r');  
    // add errror 
   while(fgets( line, 255 , fp_sensor_map) != NULL){
       uint32_t key; 
       uint16_t sensor_id; 
       uint16_t room_id; 
       char** entries = &((hash_table*)map)->entries; 
        
       sscanf(line,"%hu %hu\n", &room_id, &sensor_id); 
       uint32_t index = hash_key(sensor_id); 
       if(entries[index] == NULL){
            datamgr_table_entry* entry = malloc(sizeof(datamgr_table_entry)); 
            ptr = (datamgr_table_entry*)(entries[index]); 
            ptr->list = dpl_create(datamgr_element_copy,datamgr_element_free,datamgr_element_compare ); 
            ptr->key = sensor_id; 
            ptr->current_average = 0 ;
            ptr->room_id = room_id; 
            ptr->running_value_index = 0; 
            ptr->running_average = malloc(sizeof(double)* 5); 
            for(int i = 0; i < 5; i++){
                ptr->running_average[i] = -273; 
            }
            entries[index] = ptr; 
       }
       while(entries[index] != NULL){
           index++; 
       }
         datamgr_table_entry* entry = malloc(sizeof(datamgr_table_entry)); 
            ptr = (datamgr_table_entry*)(entries[index]); 
            ptr->list = dpl_create(datamgr_element_copy,datamgr_element_free,datamgr_element_compare ); 
            ptr->key = sensor_id; 
            ptr->current_average = 0 ;
            ptr->room_id = room_id; 
            ptr->running_value_index = 0; 
            ptr->running_average = malloc(sizeof(double)* 5); 
            for(int i = 0; i < 5; i++){
                ptr->running_average[i] = -273; 
            }
            entries[index] = ptr; 
   }


    if(fclose(fp_sensor_map) != 0){
        #ifdef DEBUG
        fprintf(stderr, "File wasn't closed. "); 
        fflush(stderr); 
        #endif
        return ; 
    }
}

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
int datamgr_add_table_entry(void* map, sensor_data_t* data){
    // No mutex needed because no
    // steps: check if key exists if not create it 
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
            entry = (sbuffer_table_entry*)(get_entry_by_index((hash_table*)map, index)); 
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
    return 0; 
}
void datamgr_free_entry(void*entry){
    datamgr_table_entry*  ptr = (datamgr_table_entry*)entry; 
    dpl_free(ptr->list, 1); 
    free(ptr->running_average); 
    ptr->list = NULL ;
    ptr->running_average = NULL; 
    free(ptr); 
}

//datamgr end
uint32_t hash_key(uint32_t id){
    uint32_t hash = 2166136261; //32 bit offset
    uint32_t FNV_prime = 16777619 ;
    return (hash*FNV_prime)^ id;
    /*
   hash = offset_basis
    for each octet_of_data to be hashed
     hash = hash * FNV_prime
     hash = hash xor octet_of_data
    return hash

32 bit offset_basis = 2166136261
32 bit FNV_prime = 224 + 28 + 0x93 = 16777619*/ 
}