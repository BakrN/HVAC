#include "datamgr.h"
#include "connmgr.h"
#include "sensor_db.h" 

#include <memory.h>

void main(int argc, char* argv[]){
    pid_t logger_process = fork(); 
    if(logger_process == 0){
        log_init(); 
    }
    pthread_t working_threads[3]; 
    int data_conn_pipefds[2]; 
    if(!pipe2(data_conn_pipefds, O_NONBLOCK)){
        //log faile dto open connection between connmgr and datamgr; 
    }
    int port = 1234;
    sbuffer_t* buffer1;
    sbuffer_init(&buffer1); 
    //connmgr_args
    char* connmgr_args = malloc(2*sizeof(int) + sizeof(sbuffer_t*)); 
    memcpy(connmgr_args, &port, sizeof(int)); 
    memcpy((void*)(connmgr_args+sizeof(int)),data_conn_pipefds, sizeof(int));
    memcpy((void*)(connmgr_args+2*sizeof(int)),&buffer1, sizeof(sbuffer_t*));
    //
    //datamgr_args
    FILE* fp_sensor_map = fopen("room_sensor.map", "r"); // closed in datamgr_init
    char* datamgr_args = malloc(sizeof(int) + sizeof(FILE*) + sizeof(sbuffer_t*));
    memcpy(datamgr_args, &data_conn_pipefds[1],sizeof(int) ); 
    memcpy((void*)(datamgr_args+sizeof(int)), &fp_sensor_map, sizeof(FILE*)); 
    memcpy((void*)(datamgr_args+sizeof(int) + sizeof(FILE*)),&buffer1, sizeof(sbuffer_t*));
    //

    pthread_create(&(working_threads[0]), NULL, strmgr_init_connection, NULL); 
    pthread_create(&(working_threads[1]), NULL,datamgr_init, datamgr_args); 
    pthread_create(&(working_threads[2]), NULL, connmgr_init, connmgr_args); 

    void* retvals [3]; 
    for(int i= 0; i < 3; i++){
        pthread_join(working_threads[i], retvals[i]); 
    }
    
    free(connmgr_args); 
    free(datamgr_args); 

    sbuffer_free(&buffer1);
    kill(logger_process, SIGINT) ; 

    wait(NULL); 

}