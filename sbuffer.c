/**
 * \author Abubakr Ehab Samir Nada
 */
#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"
#include <memory.h>

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */


/**
 * a structure to keep track of the buffer
 */


int sbuffer_init(sbuffer_t **buffer) {
    sbuffer_t* ptr = malloc(sizeof(sbuffer_t)); 
    ptr->map = create_table(sbuffer_free_entry, sbuffer_add_table_entry, NULL, NULL); 
    pthread_rwlock_init(&(ptr->sbuffer_edit_mutex),NULL); 
    pthread_cond_init(&(ptr->sbuffer_element_added), NULL); 
    *buffer = ptr; 
    if (*buffer == NULL){return SBUFFER_FAILURE; } 
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    if(buffer == NULL || *buffer == NULL){
        return SBUFFER_FAILURE; 
    }

    pthread_rwlock_wrlock(&((*buffer)->sbuffer_edit_mutex)); 
    destroy_table((*buffer)->map); 
    pthread_rwlock_unlock(&((*buffer)->sbuffer_edit_mutex)); 
    free(*buffer); 
    *buffer = NULL; 
    return SBUFFER_SUCCESS; 
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    // NOT NEEDED SINCE BUFFER IT IS AUTOMATICALLY cleaned up 
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    // Packaging data addresses
    
    //end
    pthread_rwlock_wrlock(&(buffer->sbuffer_edit_mutex)); 
    add_entry(buffer->map,(void*)data); 
    pthread_rwlock_unlock(&(buffer->sbuffer_edit_mutex)); 
    pthread_cond_broadcast(&(buffer->sbuffer_element_added)); 
    
}


int sbuffer_add_table_entry(void* map, void* arg){
    
    sensor_data_t* data = (sensor_data_t*)arg; 
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
               
                while(!(entry->list->head)){
                    dpl_remove_at_index(entry->list, 0,1 ); 
                } 
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

sbuffer_table_entry* get_next(sbuffer_t* buffer, ENTRY_TYPE type){
    pthread_rwlock_rdlock(&(buffer->sbuffer_edit_mutex)); 

    hash_table* map = buffer->map; 
   
    long* entries =  map->entries;  
    sbuffer_table_entry* entry; 
    for(int i =0; i < HASH_TABLE_SIZE; i++){
        entry = (sbuffer_table_entry*) entries[i];      
        if(type == DATA_ENTRY){
            if(entry != NULL && entry->tbr_datamgr> 0){
                
                buffer->datamgr_iterator= entry; 
              pthread_rwlock_unlock(&(buffer->sbuffer_edit_mutex)); 
                return entry;
            }
            buffer->datamgr_iterator = NULL; 
            return NULL; 
        }
        else{
            if(entry != NULL  && entry->tbr_strmgr> 0){
                buffer->strmgr_iterator = entry; 
                pthread_rwlock_unlock(&(buffer->sbuffer_edit_mutex)); 
                return entry;
            }
            buffer->strmgr_iterator = NULL; 
            return NULL; 
        }
    }
  
       
}

void sbuffer_update_entry(sbuffer_t* buffer, sbuffer_table_entry* entry, ENTRY_TYPE type , int count){
    if (count>0){

        pthread_rwlock_wrlock(&buffer->sbuffer_edit_mutex); 
        if(type==DATA_ENTRY){
            entry->tbr_datamgr -= count ; 
            
        }
        else{
            entry->tbr_strmgr-=count; 
        }
        if(entry->tbr_strmgr == 0 && entry->tbr_datamgr == 0){
            // you can free memory 
            dpl_free(&entry->list, 1); 
        }
        pthread_rwlock_unlock(&buffer->sbuffer_edit_mutex); 

    }
}