#include <hashtable.h> 
hash_table* create_table(void)  {
    hash_table* mp = malloc(sizeof(hash_table));
    mp->entries = calloc(HASH_TABLE_SIZE, sizeof(table_entry)); 
    if(mp->entries == NULL){
        free(mp); 
        return NULL; 
    }

    for(int i = 0; i < HASH_TABLE_SIZE; i++){
        mp->entries[i].runningvalues = malloc(8 * 5); 
        mp->entries[i].sensor_id = HASH_TABLE_SIZE+1; 
        mp->entries[i].runnig_value_index= 0; 

        for(int j = 0; j < 5; j++){
            mp->entries[i].runningvalues[j] = -273; 
        }
    }
    mp->capacity = HASH_TABLE_SIZE; 
    mp->count = 0; 
    return mp; 
}

void destroy_table(hash_table* map){
    for(int i = 0; i < map->capacity; i++){
        free(map->entries[i].runningvalues); 
    }
    free(map->entries); 
    free(map); 
    map == NULL; 
}      

/*


    hash = offset_basis
    for each octet_of_data to be hashed
     hash = hash * FNV_prime
     hash = hash xor octet_of_data
    return hash

32 bit offset_basis = 2166136261
32 bit FNV_prime = 224 + 28 + 0x93 = 16777619*/