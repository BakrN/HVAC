#include "connmgr.h"
#include <memory.h>
void main(void){
    pid_t logger_process = fork(); 
    if(logger_process == 0){
        log_start(); 
    }
    else{
        
    pthread_t connmgr_thread; 
    int port = 1234; 
    sbuffer_t* test;
    sbuffer_init(&test); 
    char* connmgr_args = malloc(sizeof(int) + sizeof(sbuffer_t*)); 
    memcpy(connmgr_args, &port, sizeof(int)); 
    memcpy((void*)(connmgr_args+sizeof(int)),&test, sizeof(sbuffer_t*)); 
    pthread_create(&connmgr_thread, NULL , &connmgr_init, connmgr_args); 
    void* retval; 
    pthread_join(connmgr_thread, &retval); 
    

    //free args 
    free(connmgr_args); 
    sbuffer_free(&test); 
    kill(logger_process, SIGINT); 
    wait(NULL); 
    }
}