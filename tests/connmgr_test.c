#include "connmgr.h"


void main(void){
    pid_t logger_process = fork(); 
    if(logger_process == 0){
        log_init(); 
    }
    else{
        
    pthread_t connmgr_thread; 
    int port = 1234; 
    pthread_create(&connmgr_thread, NULL , &connmgr_init, &port); 
    void* retval; 
    pthread_join(connmgr_thread, &retval); 
    
    kill(logger_process, SIGINT); 
    wait(NULL); 
    }
}