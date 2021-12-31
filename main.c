
#define _GNU_SOURCE
#include "datamgr.h"
#include "connmgr.h"
#include "sensor_db.h"
#include "logger.h" 
#include <unistd.h>
#include <memory.h>
  #include <sys/types.h>
  #include <sys/wait.h>

// NOTE reader threads should start first and subscribe to sbuffer then write thread can start; 
int main(int argc, char* argv[]){
        int port;
         if (argc != 2) {
        exit(EXIT_SUCCESS);
    } else {
        // to do: user input validation!
        port = atoi(argv[1]);

    }
    logger_t* logger = log_init(); 
 
    pid_t logger_process = fork(); 
    if(logger_process == 0){
        log_start(logger); 
    }
    logger->w_pipefd = open("logFifo", O_WRONLY  ) ; // blocking
    pthread_t working_threads[3]; 
    int data_conn_pipefds[2]; 
    if(!pipe2(data_conn_pipefds, O_NONBLOCK)){
        //log faile dto open connection between connmgr and datamgr; 
    }
    
    sbuffer_t* buffer1;
    sbuffer_init(&buffer1); 
    //connmgr_args
    conn_args* connmgr_args = malloc(sizeof(conn_args)); 
    connmgr_args->port_number = port; 
    connmgr_args->pipefd = data_conn_pipefds[0]; 
    connmgr_args->buffer =(void*) buffer1;  
    connmgr_args->logger = (void*) logger; 

    //
    //datamgr_args
    datamgr_args* data_args = malloc(sizeof(datamgr_args)); 
    FILE* fp_sensor_map = fopen("room_sensor.map", "r"); // closed in datamgr_init
    data_args->pipefd = data_conn_pipefds[1]; 
    data_args->fp_sensor_map = fp_sensor_map; 
    data_args->buffer = buffer1; 
    data_args->logger = (void*) logger; 
    data_args->reader_thread_id = 0;


    // strgmgr_args 
    strgmgr_args* db_args = malloc(sizeof(strgmgr_args)) ; 
    
    db_args->buffer = (void*)buffer1; 
    db_args->clear_flag = 1; 
    db_args->reader_thread_id = 1; 
    db_args->logger = (void*)logger;
    
    pthread_create(&(working_threads[0]), NULL, strgmgr_init, db_args); 


    pthread_create(&(working_threads[1]), NULL,datamgr_init, data_args); 
    pthread_create(&(working_threads[2]), NULL, connmgr_init, connmgr_args); 

    void* retvals [3]; 
    
    for (int i = 0; i <3 ; i++){
        pthread_join(working_threads[i], &retvals[i]); 
    }
    printf("JOINED THREADS\n"); 
    free(connmgr_args); 
    free(data_args); 
    free(db_args); 

    sbuffer_free(&buffer1);

    log_destroy(logger); 
    kill(logger_process, SIGINT) ; 
 
    wait(NULL); 
    return 0 ; 
}
