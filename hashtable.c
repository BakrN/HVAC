#include "hashtable.h" 



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


