#include "../hashtable.h"

// HASHTABLES FIXED

void main(void){
    hash_table* map = create_table(sbuffer_free_entry, sbuffer_add_table_entry, NULL , NULL); 
  
    sensor_data_t* data1 = malloc(sizeof(sensor_data_t)); 
    data1->id = 10; 
    data1->ts = 302014; 
    data1->value = 24; 
    sensor_data_t* data2 = malloc(sizeof(sensor_data_t)); 
    data2->id = 10; 
    data2->ts = 302014; 
    data2->value = 25; 
    add_entry(map,data1 ); 
    add_entry(map, data2); 
    sbuffer_table_entry* returned = get_next(map,DATA_ENTRY ); 
    sensor_data_t* returned_sensor = (sensor_data_t*)dpl_get_element_at_index(returned->list, 1); 
    // check and see if return 
    printf("Returned: ID: %d, TIME: %ld, VALUE: %f\n", returned_sensor->id, returned_sensor->ts, returned_sensor->value); 
    printf("Original: ID: %d, TIME: %ld, VALUE: %f\n", data1->id, data1->ts, data1->value); 
    destroy_table(map); 
} 
