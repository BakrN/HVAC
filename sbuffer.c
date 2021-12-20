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

int sbuffer_init(sbuffer_t **buffer){
    sbuffer_t *ptr = malloc(sizeof(sbuffer_t));
    ptr->iterators = NULL;
    ptr->reader_thread_count = 0;
    ptr->terminate_reader_threads = calloc(1, 1);

    ptr->map = umap_create(sbuffer_free_entry, sbuffer_add_table_entry, NULL, NULL);
    pthread_mutex_init(&(ptr->sbuffer_edit_mutex), NULL);
    pthread_cond_init(&(ptr->sbuffer_element_added), NULL);

    *buffer = ptr;
    if (*buffer == NULL)
    {
        return SBUFFER_FAILURE;
    }

    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer){
    if (buffer == NULL || *buffer == NULL)
    {
        return SBUFFER_FAILURE;
    }

    pthread_mutex_lock(&((*buffer)->sbuffer_edit_mutex));
    umap_destroy((*buffer)->map);
    pthread_mutex_unlock(&((*buffer)->sbuffer_edit_mutex));
    for (int i = 0 ; i < (*buffer)->reader_thread_count; i++){// free iterators
        free((*buffer)->iterators[i]); 
    }
    free((*buffer)->iterators); 
    free((*buffer)->terminate_reader_threads); 
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

char sbuffer_wait_for_data(sbuffer_t *buffer, int thread_id){
    
    sbuffer_iterator *iterator = NULL;
    for (int i = 0; i < buffer->reader_thread_count; i++)
    {
        if (buffer->iterators[i]->thread_id == thread_id)
        {
            iterator = buffer->iterators[i];
            break;
        }
    }
    if (!iterator)
    {
        // reader thread hasn't subscribed ;
        return 1;
    }
    iterator->entry = get_next(buffer, thread_id); 
    pthread_mutex_lock(&(buffer->sbuffer_edit_mutex));

    while (*(buffer->terminate_reader_threads) == 0 && iterator->entry == NULL)
    {
        
        pthread_cond_wait(&(buffer->sbuffer_element_added), &(buffer->sbuffer_edit_mutex)); // wakes up with locked mutex
        iterator->entry = get_next(buffer, thread_id);
        
    }
    pthread_mutex_unlock(&(buffer->sbuffer_edit_mutex));
    
    if (*(buffer->terminate_reader_threads) && iterator->entry == NULL)
    { // woken up due to termination

        
        return 1;
    }

    return 0;
}
int sbuffer_get_entry_tbr(sbuffer_t* buffer, sbuffer_table_entry* entry, int thread_id){
    for (int i = 0; i <buffer->reader_thread_count; i++){
        if(entry->to_be_read[i]->thread_id == thread_id) {
            return entry->to_be_read[i]->tbr_count; 
        }
    }
    return -1 ; 
}
void sbuffer_reader_subscribe(sbuffer_t *buffer, int thread_id)
{
    pthread_mutex_lock(&(buffer->sbuffer_edit_mutex)); 
    buffer->reader_thread_count++;
    buffer->iterators =  realloc(buffer->iterators, buffer->reader_thread_count * sizeof(sbuffer_iterator *));
    (buffer->iterators)[buffer->reader_thread_count - 1] = malloc(sizeof(sbuffer_iterator));
    (buffer->iterators)[buffer->reader_thread_count - 1]->entry = NULL;
    (buffer->iterators)[buffer->reader_thread_count - 1]->thread_id = thread_id;

    pthread_mutex_unlock(&(buffer->sbuffer_edit_mutex)); 
}

void sbuffer_reader_unsubscribe(sbuffer_t *buffer, int thread_id)
{
    // lock mutex so info is added until the reader thread's data is deleted 
    pthread_mutex_lock(&(buffer->sbuffer_edit_mutex)); 
    sbuffer_table_entry* entry; 
    buffer->reader_thread_count--; 
    for (int i = 0 ; i < buffer->map->capacity; i++){
        entry = (sbuffer_table_entry*)umap_get_entry_by_index(buffer->map, i); 
        if(entry){
     
            for (int j = 0 ; j < entry->tbr_array_size; j++){
                if (entry->to_be_read[i]->thread_id == thread_id){
                    for (int k = j; k < entry->tbr_array_size-1; k++){
                        entry->to_be_read[k] = entry->to_be_read[k+1]; 

                    }
                    entry->tbr_array_size--; 
                    free(entry->to_be_read[entry->tbr_array_size]); 
                    entry->to_be_read = realloc(entry->to_be_read, sizeof(sbuffer_entry_toberead* )*(entry->tbr_array_size)); 
                    break; 
                }
            }
  
        }
    }
    pthread_mutex_unlock(&(buffer->sbuffer_edit_mutex)); 
}

typedef struct
{
    sensor_data_t *data;
    sbuffer_t *buffer;
  

} sbuffer_table_args;
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data)
{
    // Packaging data addresses

    //end
    sbuffer_table_args *args = malloc(sizeof(sbuffer_table_args));
    args->buffer = buffer;
    args->data = data;
    pthread_mutex_lock(&(buffer->sbuffer_edit_mutex));
    umap_addentry(buffer->map, args);
    pthread_cond_broadcast(&(buffer->sbuffer_element_added)); // wake up other threads if they're asleep
    pthread_mutex_unlock(&(buffer->sbuffer_edit_mutex));
    free(args); 
    return 0;
}

int sbuffer_add_table_entry(void *map, void *arg)
{

    sensor_data_t *data = ((sbuffer_table_args *)(arg))->data;
    sbuffer_t *buffer = ((sbuffer_table_args *)(arg))->buffer;

    // also pass buffer as arg
    // steps: check if key exists if not create it /*
    uint32_t index = hash_key(data->id);
    // searching if entry already exists (method of adding an entry is linear)
    if (umap_get_entry_by_index((hash_table *)map, index) != NULL)
    {

        sbuffer_table_entry *entry = (sbuffer_table_entry *)(umap_get_entry_by_index((hash_table *)map, index));
        if (entry->key == data->id)
        
        {
            dpl_insert_at_index(entry->list, data, 0, 0); // not copied
            for (int i = 0; i < buffer->reader_thread_count; i++)
            {
                // add all reader thread ids and count to new entry
                if (!(buffer->iterators[i])->entry)
                {
                    // iterator pointing to nothing
                    buffer->iterators[i]->entry = entry;
                }
                entry->to_be_read[i]->tbr_count += 1;
            }
            //unlock
            return 0; // success
        }
        uint32_t search = index + 1;
        while (search != index)
        {
            // search for it linearly
            entry = (sbuffer_table_entry *)(umap_get_entry_by_index(map, index));
            if (entry == NULL)
            {
                // create
                entry = malloc(sizeof(sbuffer_table_entry));
                entry->tbr_array_size = 0; 
                entry->key = data->id;
                entry->list = dpl_create(NULL, sbuffer_listelement_free, NULL);
                dpl_insert_at_index(entry->list, data, 0, 0);

                entry->to_be_read = malloc(buffer->reader_thread_count * sizeof(sbuffer_entry_toberead *));
                for (int i = 0; i < buffer->reader_thread_count; i++)
                {
                    // add all reader thread ids and count to new entry
                    if (!(buffer->iterators[i])->entry)
                    {
                        // iterator pointing to nothing
                        buffer->iterators[i]->entry = entry;
                    }
                    entry->to_be_read[i] = malloc(sizeof(sbuffer_entry_toberead));
                    entry->to_be_read[i]->thread_id = buffer->iterators[i]->thread_id;
                    entry->to_be_read[i]->tbr_count = 1;
                    entry->tbr_array_size++; 
                }
                long *entries = (long *)((hash_table *)map)->entries;
                entries[index] = (long)(void *)entry;
                return 0;
            }
            // check if sensor id already exists
            else if (entry->key == data->id)
            {
                // write lock/ mutex

                // Don't need mutex if only adding to index 0, just make sure you aren't checking prev
                dpl_insert_at_index(entry->list, data, 0, 0); // not copied

                for (int i = 0; i < buffer->reader_thread_count; i++)
                {
                    // add all reader thread ids and count to new entry
                    if (!(buffer->iterators[i])->entry)
                    {
                        // iterator pointing to nothing
                        buffer->iterators[i]->entry = entry;
                    }
                    entry->to_be_read[i]->tbr_count += 1;
                    //unlock
                    return 0; // success
                }

                if (search++ == HASH_TABLE_SIZE)
                {
                    search = 0; // wrap back down
                }
            }
        }

            // not found
            // this shouldn't happen
            return -1;
        }

        // create new entry
        //
        sbuffer_table_entry *new_entry = malloc(sizeof(sbuffer_table_entry));
        new_entry->tbr_array_size = 0; 
        new_entry->key = data->id;
        new_entry->list = dpl_create(NULL, sbuffer_listelement_free, NULL);
        dpl_insert_at_index(new_entry->list, (void *)data, 0, 0);

        new_entry->to_be_read = malloc(buffer->reader_thread_count * sizeof(sbuffer_entry_toberead *));
        for (int i = 0; i < buffer->reader_thread_count; i++)
        {
            // add all reader thread ids and count to new entry
            if (!(buffer->iterators[i])->entry)
            {
                // iterator pointing to nothing
                buffer->iterators[i]->entry = new_entry;
            }
            new_entry->to_be_read[i] = malloc(sizeof(sbuffer_entry_toberead));
            new_entry->to_be_read[i]->thread_id = buffer->iterators[i]->thread_id;
            new_entry->to_be_read[i]->tbr_count = 1;
            new_entry->tbr_array_size++; 
        }

        long *entries = (long *)((hash_table *)map)->entries;
        printf("Added entry at %d\n", index);
        entries[index] = (long)(void *)new_entry;
        return 0;
    }

    void sbuffer_free_entry(void *entry)
    {
        sbuffer_table_entry *ptr = (sbuffer_table_entry *)entry;
        dpl_free(&ptr->list, 1);
        ptr->list = NULL;
        for(int i =0 ;i < ptr->tbr_array_size; i++ ){
            //ptr->to_be_read[]
            free(ptr->to_be_read[i]) ; 
        }
        free(ptr->to_be_read); 
        free(ptr);
        ptr = NULL;
    }

    void sbuffer_listelement_free(void **element)
    {
        free(*element); // frees sensor_data_id pointer;
        *element = NULL;
    }

    sbuffer_table_entry *get_next(sbuffer_t * buffer, int thread_id)
    {
        sbuffer_iterator* ptr = NULL ; // iterator of specific thread_id; 
        for(int i = 0; i < buffer->reader_thread_count; i++){
            if (buffer->iterators[i]->thread_id == thread_id){
                ptr = buffer->iterators[i]; 
                      if(ptr->entry && sbuffer_get_entry_tbr(buffer, ptr->entry, thread_id)>0){ // iterator was se when entering new data into buffer 
            return ptr->entry; 
        }
                break; 
            }
        } 
     
        if(ptr){ // reader thread id found 
  
 

             // iterator ptr needs data because it's current null 
    hash_table* map = buffer->map; 
   
        long* entries =  map->entries;  
        sbuffer_table_entry* entry; 
        sbuffer_entry_toberead* tbr =NULL; //thread specific count
        for(int i =0; i < HASH_TABLE_SIZE; i++){
      
        entry = (sbuffer_table_entry*) entries[i]; 
    
        if(entries[i]!=0){ // sbuffer table entry not null 
            for (int i = 0; i < buffer->reader_thread_count; i++){
                if(entry->to_be_read[i]->thread_id == thread_id){
                    tbr = entry->to_be_read[i]; 
                }
        }
        if(tbr){
            if(tbr->tbr_count > 0 ){
                ptr->entry = entry; 
                return ptr->entry; 
            }
            } 
        } 
        }
        ptr->entry = NULL ;
        }
        
        return NULL; // no data found 
    }
//old sbuffer updat
/*  void sbuffer_update_entry(sbuffer_t * buffer, sbuffer_table_entry * entry, int thread_id, int count){
        
        if (count > 0)
        {
            sbuffer_entry_toberead* tbr = NULL ; // pointer to tbr counter for specific thread id 
            for (int i = 0 ; i < buffer->reader_thread_count; i++){
                if(entry->to_be_read[i]->thread_id == thread_id){
                    tbr = entry->to_be_read[i]; 
                    break; 
                }
            }
            if(tbr){
            pthread_mutex_lock(&buffer->sbuffer_edit_mutex);
    
                tbr->tbr_count -= count;
            char remaining_threads_to_read = buffer->reader_thread_count; 
            for(int i =0 ; i < buffer->reader_thread_count; i++){
                if(entry->to_be_read[i]->tbr_count ==0){
                    remaining_threads_to_read--; 
                }
            }
            if (remaining_threads_to_read==0)
            {
                // you can free list memory
                while (entry->list->head)
                { // remove elements
                    entry->list = dpl_remove_at_index(entry->list, 0, 1);
                }
                // 
            }

            pthread_mutex_unlock(&buffer->sbuffer_edit_mutex);
        }
    }
    }*/




    void sbuffer_update_entry(sbuffer_t * buffer, sbuffer_table_entry * entry, int thread_id, int count){
        
       
            sbuffer_entry_toberead* tbr = NULL ; // pointer to tbr counter for specific thread id 
            for (int i = 0 ; i < buffer->reader_thread_count; i++){
                if(entry->to_be_read[i]->thread_id == thread_id){
                    tbr = entry->to_be_read[i]; 
                    break; 
                }
            }

            pthread_mutex_lock(&buffer->sbuffer_edit_mutex);
            if(tbr){
            
    
                tbr->tbr_count -= count;
            }
            char remaining_threads_to_read = buffer->reader_thread_count; 
            for(int i =0 ; i < buffer->reader_thread_count; i++){
                if(entry->to_be_read[i]->tbr_count ==0){
                    remaining_threads_to_read--; 
                }
            }
            if (remaining_threads_to_read==0)
            {
                // you can free list memory
                while (entry->list->head)
                { // remove elements
                    entry->list = dpl_remove_at_index(entry->list, 0, 1);
                }
                // 
            }

            pthread_mutex_unlock(&buffer->sbuffer_edit_mutex);
        
    
    }
void sbuffer_wakeup_readerthreads(sbuffer_t* buffer){
     pthread_mutex_lock(&(buffer->sbuffer_edit_mutex)); 
    pthread_cond_broadcast(&(buffer->sbuffer_element_added )); // wake up other threads if they're asleep 
    pthread_mutex_unlock(&(buffer->sbuffer_edit_mutex)); 
}