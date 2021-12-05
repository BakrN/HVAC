/**
 * \author Abubakr Ehab Samir Nada
 */
#define _GNU_SOURCE 
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
     ptr->datamgr_iterator = NULL; 
    ptr->strmgr_iterator= NULL; 
    ptr->map = create_table(sbuffer_free_entry, sbuffer_add_table_entry, NULL, NULL); 
    pthread_mutex_init(&(ptr->sbuffer_edit_mutex), NULL ); 
    pthread_cond_init(&(ptr->sbuffer_element_added), NULL); 
    *buffer = ptr; 
    if (*buffer == NULL){return SBUFFER_FAILURE; } 
 
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    if(buffer == NULL || *buffer == NULL){
        return SBUFFER_FAILURE; 
    }

    pthread_mutex_lock(&((*buffer)->sbuffer_edit_mutex)); 
    destroy_table((*buffer)->map); 
    pthread_mutex_unlock(&((*buffer)->sbuffer_edit_mutex)); 
    free(*buffer); 
    *buffer = NULL; 
    return SBUFFER_SUCCESS; 
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    // NOT NEEDED SINCE BUFFER IT IS AUTOMATICALLY cleaned up 
}
typedef struct {
    sensor_data_t* data; 
    sbuffer_t* buffer; 
} sbuffer_table_args; 
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    // Packaging data addresses
    
    //end
    sbuffer_table_args* args = malloc(sizeof(sbuffer_table_entry)); 
    args->buffer = buffer; 
    args->data = data; 
    pthread_mutex_lock(&(buffer->sbuffer_edit_mutex)); 
             
        add_entry(buffer->map,args); 

    pthread_cond_broadcast(&(buffer->sbuffer_element_added )); // wake up other threads if they're asleep 
    pthread_mutex_unlock(&(buffer->sbuffer_edit_mutex)); 

    return 0 ;  
    }


int sbuffer_add_table_entry(void* map, void* arg){
    
    sensor_data_t* data = ((sbuffer_table_args*)(arg))->data; 
    sbuffer_t* buffer = ((sbuffer_table_args*)(arg))->buffer;

    // also pass buffer as arg 
    // steps: check if key exists if not create it /*
    uint32_t index = hash_key(data->id) ; 
    // searching if entry already exists (method of adding an entry is linear)
    if( get_entry_by_index((hash_table*)map, index) != NULL )
    {
        
        sbuffer_table_entry* entry = (sbuffer_table_entry*)(get_entry_by_index((hash_table*)map, index)); 
        if(entry->key == data->id){

        
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
         
                entry->tbr_datamgr = 1; 
                entry->tbr_strmgr = 1;
                if(!(buffer->strmgr_iterator)){buffer->strmgr_iterator = entry; }
                if(!(buffer->datamgr_iterator)){buffer->datamgr_iterator= entry; }
                long* entries = (long*)((hash_table*)map)->entries;  
                entries[index] = (long)(void*)entry; 
                return 0;  
            }
            // check if sensor id already exists
            else if (entry->key == data->id){
                            // write lock/ mutex
           

            // Don't need mutex if only adding to index 0, just make sure you aren't checking prev
            dpl_insert_at_index(entry->list,data, 0 ,0); // not copied 
            
        
            entry->tbr_datamgr +=1; 
            entry->tbr_strmgr += 1; 
                if(!(buffer->strmgr_iterator)){buffer->strmgr_iterator = entry; }
                if(!(buffer->datamgr_iterator)){buffer->datamgr_iterator= entry; }
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
    printf("Added entry at %d\n", index); 
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

    if(buffer->datamgr_iterator && type == DATA_ENTRY){return buffer->datamgr_iterator; }
    if(buffer->strmgr_iterator&& type == STORE_ENTRY){return buffer->strmgr_iterator; }

    hash_table* map = buffer->map; 
   
    long* entries =  map->entries;  
    sbuffer_table_entry* entry; 
    for(int i =0; i < HASH_TABLE_SIZE; i++){
      
        entry = (sbuffer_table_entry*) entries[i];      
        if(entries[i]!=0){
            if(type == DATA_ENTRY && entry->tbr_datamgr >0 ){
                buffer->datamgr_iterator = entry; 
                return entry; 
            }
            else if(type == STORE_ENTRY && entry->tbr_strmgr >0){
                buffer->strmgr_iterator= entry; 
                return entry; 
            }
        }  
    }

               
    if(type==DATA_ENTRY){
         printf("returned datav null\n")     ; 
     buffer->datamgr_iterator = NULL; 
            return NULL; 
    }
    if(type == STORE_ENTRY){
         printf("returned storage null\n")     ; 
                 buffer->strmgr_iterator = NULL; 
            return NULL; 
    }
       
}

void sbuffer_update_entry(sbuffer_t* buffer, sbuffer_table_entry* entry, ENTRY_TYPE type , int count){
    if (count>0){

        pthread_mutex_lock(&buffer->sbuffer_edit_mutex); 
        if(type==DATA_ENTRY){
            entry->tbr_datamgr -= count ; 
            
        }
        else{
            entry->tbr_strmgr-=count; 
        }
        if(entry->tbr_strmgr == 0 && entry->tbr_datamgr == 0){
            // you can free memory 
            while(entry->list->head){ // remove elements
                entry->list = dpl_remove_at_index(entry->list,0,1); 
            }
        }
        pthread_mutex_unlock(&buffer->sbuffer_edit_mutex); 

    }
}