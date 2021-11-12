#include "datamgr.h"
#include "connmgr.h"
#include "sensor_db.h" 




void main(int argc, char* argv[]){
    pid_t logger_process = fork(); 
    if(logger_process == 0){
        log_init(); 
    }
    pthread_t working_threads[3]; 
    int port = 1234;
    sbuffer_t* buffer1;
    sbuffer_init(&buffer1); 
    char* connmgr_args = malloc(sizeof(int) + sizeof(sbuffer_t*)); 
    memcpy(connmgr_args, &port, sizeof(int)); 
    memcpy((void*)(connmgr_args+sizeof(int)),&buffer1, sizeof(sbuffer_t*));
    pthread_create(&(working_threads[0]), NULL, strmgr_init_connection, NULL); 
    pthread_create(&(working_threads[1]), NULL, datamgr_parse_sensor_files, NULL); 
    pthread_create(&(working_threads[2]), NULL, connmgr_init, connmgr_args); 

    void* retvals [3]; 
    for(int i= 0; i < 3; i++){
        pthread_join(working_threads[i], retvals[i]); 
    }
    
    free(connmgr_args); 
    sbuffer_free(&buffer1);
    kill(logger_process, SIGINT) ; 

    wait(NULL); 

}