#include <hashtable.h> 


hash_table* create_table(void* (*initialize_entry)(void*entry),void (*add_table_entry)(void* entry) , void (*free_entry)(void*entry),uint32_t (*hash_key)(void* entry), void (*add_to_list)(sensor_data_t* data)) {

    hash_table* mp = malloc(sizeof(hash_table));
    void* ptr = calloc(HASH_TABLE_SIZE, sizeof(void*)); 
    mp->entries = &ptr;
    mp->add_table_entry = add_table_entry;  
    
    mp->free_entry = free_entry; 
    mp->capacity = HASH_TABLE_SIZE; 
    mp->count = 0; 
    char** entries = (char**) mp->entries; 
    for( int i = 0; i < HASH_TABLE_SIZE; i++){
        // entries[i] = mp->initialize_entry((void*)(entries[i])); 
        entries[i] = NULL ; // Set them NUll; 
    }
    if(mp->entries == NULL){
        free(mp); 
        return NULL; 
    }

    return mp; 
}
void* get_entry_by_index(hash_table* map, uint32_t index){
    char** entries = (char**) map->entries; 
    return (void*)map->entries[index]; 
}
void destroy_table(hash_table* map){
    char** entries = (char**) map->entries;
    for(int i = 0; i < map->capacity; i++){
        if(entries[i] != NULL){
            map->free_entry((void*)entries[i]);
            entries[i] = NULL; 
        } 
    }
    free(map->entries); 
    free(map); 
    map == NULL; 
}      
int add_table_entry(hash_table*map, sensor_data_t* data){
    return map->add_table_entry(data) ; 
}
int sbuffer_add_table_entry(hash_table* map, sensor_data_t* data){
    
    // steps: check if key exists if not create it 
    uint32_t index = hash_key(data->id) ; 
    // searching if entry already exists (method of adding an entry is linear)
    if( get_entry_by_index(map, index) != NULL )
    {
        sbuffer_table_entry* entry = (sbuffer_table_entry*)(get_entry_by_index(map, index)); 
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
                entry->list = dpl_create(NULL, NULL, NULL); 
                dpl_insert_at_index(entry->list, data, 0, 0); 
                // no mutex needed because no entry can acess; it 
                entry->tbr_datamgr = 1; 
                entry->tbr_strmgr = 1;
         
                 //unlock
                char** entries = (char**) map->entries; 
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
    new_entry->list = dpl_create(NULL, NULL, NULL); 
    dpl_insert_at_index(new_entry->list, data, 0, 0); 
    // no mutex needed because no entry can acess; it 
    new_entry->tbr_datamgr = 1; 
    new_entry->tbr_strmgr = 1;
     
    //unlock
    char** entries = (char**) map->entries; 
    entries[index] = (void*)new_entry; 
    return 0; 
}

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