#include "logger.h"
#include <ctype.h> 
#include <string.h> 

void signal_handler(int signal){
    if (signal == SIGINT){
        printf("Logger Process terminated\n"); 
        log_destroy(); 
    }
}
char* trim_string(char* str, size_t len){
    // removes spaces at the end; 
    while(isspace(str[len-1])) --len; 
    char* new_msg = strndup(str,len); 
    free(str);
    return new_msg;  
}
int log_init(){
    signal(SIGINT, signal_handler); 
    if(mkfifo("gateway.log", 0777) == -1){
        if(errno != EEXIST){
            printf("failed to create log process\n"); 
            return -1; 
        }
    }
    log_fd = open("gateway.log", O_RDONLY ) ;

    
    //the file position is advanced by that number after reading 

    char* buffer; 
        while(1){
            if(read(log_fd, buffer, 200) <= 0){
               // nothing to read
            }
            else{
                buffer = trim_string(buffer, 200); 
                printf("%s", buffer) ;
                free(buffer); 
                buffer = NULL; 
            }
        }
        log_destroy(); 
        return 0; 
}

void log_event(int write_fd, log_msg* event){

//writes of data with size 4096 is not interleaved 

    char* test_message; 
    
    asprintf(&test_message, "%lu %ld: %s\n", event->sequence_number, event->timestamp, event->message) ; 

    write(log_fd, test_message, strlen(test_message)); 
  
    free(test_message); 
}


void log_destroy(){
   
    close(log_fd); 
    exit(1); 
}

