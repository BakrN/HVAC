#include "sensor_db.h"
#include "logger.h"
#include <wait.h>
#include <signal.h>


void main(void){
 
    pid_t logger_process = fork(); 
    if(logger_process == 0){
        log_init(); 
    }
    
    
    DBCONN* db = strmgr_init_connection( 1) ; 

   // insert_sensor(db, 13, 15, time(0)); 

    disconnect(db) ; 
 
    kill(logger_process, SIGINT) ; 

    wait(NULL); 

    }