#define _GNU_SOURCE
#include "datamgr.h"
#include "connmgr.h"
#include "sensor_db.h" 
#include <unistd.h>
#include <memory.h>
  #include <sys/types.h>
  #include <sys/wait.h>
typedef struct{

}connmgr_args ; 



typedef struct{
    int pipefd; //write
    char* TERMINATE_READER; 
    FILE* fp_sensor_map; 
}datamgr_args ; 

int main(int argc, char* argv[]){
        int port;
         if (argc != 2) {
        exit(EXIT_SUCCESS);
    } else {
        // to do: user input validation!
        port = atoi(argv[1]);

    }
    pid_t logger_process = fork(); 
    if(logger_process == 0){
        log_init(); 
    }
    pthread_t working_threads[3]; 
    int data_conn_pipefds[2]; 
    if(!pipe2(data_conn_pipefds, O_NONBLOCK)){
        //log faile dto open connection between connmgr and datamgr; 
    }
    char* TERMINATE_READER_THREADS = calloc(1, 1) ;  


    sbuffer_t* buffer1;
    sbuffer_init(&buffer1); 
    //connmgr_args
    char* connmgr_args = malloc(2*sizeof(int) + sizeof(sbuffer_t*)+sizeof(char*)); 
    memcpy(connmgr_args, &port, sizeof(int)); 
    memcpy((void*)(connmgr_args+sizeof(int)),data_conn_pipefds, sizeof(int));
    memcpy((void*)(connmgr_args+2*sizeof(int)),&buffer1, sizeof(sbuffer_t*));
    memcpy((void*)(connmgr_args+2*sizeof(int) +sizeof(sbuffer_t*)),&TERMINATE_READER_THREADS, sizeof(char*));
    //
    //datamgr_args
    FILE* fp_sensor_map = fopen("room_sensor.map", "r"); // closed in datamgr_init
    char* datamgr_args = malloc(sizeof(int) + sizeof(FILE*) + sizeof(sbuffer_t*) + sizeof(char*));
    memcpy(datamgr_args, &data_conn_pipefds[1],sizeof(int) ); 
    memcpy((void*)(datamgr_args+sizeof(int)), &fp_sensor_map, sizeof(FILE*)); 
    memcpy((void*)(datamgr_args+sizeof(int) + sizeof(FILE*)),&buffer1, sizeof(sbuffer_t*));
    memcpy((void*)(datamgr_args+sizeof(int) + sizeof(FILE*) + sizeof(sbuffer_t*)),&TERMINATE_READER_THREADS, sizeof(sbuffer_t*));
    // strgmgr_args 
    strgmgr_args* db_args = malloc(sizeof(strgmgr_args)) ; 
    db_args->terminate_thread = TERMINATE_READER_THREADS; 
    db_args->buffer = (void*)buffer1; 
    db_args->clear_flag = 1; 
    pthread_create(&(working_threads[0]), NULL, strgmgr_init, db_args); 

    pthread_create(&(working_threads[1]), NULL,datamgr_init, datamgr_args); 
    pthread_create(&(working_threads[2]), NULL, connmgr_init, connmgr_args); 

    void* retvals [3]; 
    
    pthread_join(working_threads[0], &retvals[0]); 
    pthread_join(working_threads[1], &retvals[1]); 
    pthread_join(working_threads[2], &retvals[2]); 
    printf("JOINED THREADS\n"); 
    free(connmgr_args); 
    free(datamgr_args); 
    free(db_args); 
    free(TERMINATE_READER_THREADS); 
    sbuffer_free(&buffer1);
    printf("FREED SBUFFER\n"); 
    kill(logger_process, SIGINT) ; 
    printf("KILLLEd lgoger\n");
    wait(NULL); 
    return 0 ; 
}