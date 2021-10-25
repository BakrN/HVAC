#include "hashtable.h" 
extern sbuffer_table_entry* strmgr_iterator; // used to get next packet; points to entry of sbuffer
extern sbuffer_table_entry* datamgr_iterator; // used to get next packet; points to entry of sbuffer


hash_table* create_table(void (*free_entry)(void* entry),
    int (*add_table_entry)(void* map, sensor_data_t* data), // 0 for success , -1 otherwise 
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
void* get_entry_by_index(hash_table* map, uint32_t index){

    long* entries =  (long*)map->entries; 
    return (void*)entries[index]; 
}
void destroy_table(hash_table* map){

    long* entries =  (long*) map->entries; 
    for(int i = 0; i < map->capacity; i++){
        
       if(entries[i] != (long)NULL && map->free_entry != NULL){
           #ifdef DEBUG
            printf("Entry at index: %d , freed with address: %ld\n", i, entries[i]);
            #endif 
           (*map->free_entry)((void*)entries[i]);
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
int add_entry(hash_table*map, sensor_data_t* data){

    return (*map->add_table_entry)((void*)map, data) ; 
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
//sbuffer function implementations
int sbuffer_add_table_entry(void* map, sensor_data_t* data){
    
    // steps: check if key exists if not create it /*
    uint32_t index = hash_key(data->id) ; 
    // searching if entry already exists (method of adding an entry is linear)
    if( get_entry_by_index((hash_table*)map, index) != NULL )
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
                  
                long* entries = (long*)((hash_table*)map)->entries;  

                entries[index] = (long)(void*)entry; 
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
    //
    sbuffer_table_entry* new_entry = malloc(sizeof(sbuffer_table_entry));  
    new_entry->key = data->id; 
    new_entry->list = dpl_create(NULL,sbuffer_element_free, NULL); 
    dpl_insert_at_index(new_entry->list, (void*)data, 0, 0); 
    // no mutex needed because no entry can acess; it 
    new_entry->tbr_datamgr = 1; 
    new_entry->tbr_strmgr = 1;
     
    //unlock
    long* entries = (long*)((hash_table*)map)->entries; 
    printf("%d", index); 
    entries[index] = (long)(void*)new_entry; 
    return 0; 
 
}
void sbuffer_free_entry(void*entry){
     sbuffer_table_entry* ptr = (sbuffer_table_entry*)entry; 
    dpl_free(&ptr->list, 1); 
    ptr->list = NULL ;
    free(ptr); 
    ptr = NULL; 
}
void sbuffer_element_free(void ** element){
    free(*element); // frees sensor_data_id pointer; 
    *element=  NULL; 
}

sbuffer_table_entry* get_next(hash_table* map, ENTRY_TYPE type){
    //pthread_rwlock_rdlock(&sbuffer_edit_mutex); 
   
    long* entries =  map->entries;  
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

//datamgr end

