#include "datamgr.h"
#include "connmgr.h"
#include "sensor_db.h" 




void main(void){
    pid_t logger_process = fork(); 
    if(logger_process == 0){
        log_init(); 
    }
    pthread_t working_threads[3]; 
    int port = 1234;
    pthread_create(&(working_threads[0]), NULL, strmgr_init_connection, NULL); 
    pthread_create(&(working_threads[1]), NULL, datamgr_parse_sensor_files, NULL); 
    pthread_create(&(working_threads[2]), NULL, connmgr_listen_to_port, &port); 

    void* retvals [3]; 
    for(int i= 0; i < 3; i++){
        pthread_join(working_threads[i], retvals[i]); 
    }
    

    kill(logger_process, SIGINT) ; 

    wait(NULL); 

}