#include "../datamgr.h"


void main(void){
    FILE* sensor_map = fopen("room_sensor.map", "r"); 
    FILE* sensor_data = fopen("sensor_data", "rb");
    dplist_t* list ; 
    
    datamgr_parse_sensor_files(sensor_map, sensor_data); 
    
    datamgr_free(); 
    printf("test\n"); 

}