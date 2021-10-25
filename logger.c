#include "logger.h"
#include <ctype.h> 
#include <string.h> 
char* trim_string(char* str, size_t len){
    // removes spaces at the end; 
    while(isspace(str[len-1])) --len; 
    char* new_msg = strndup(str,len); 
    free(str);
    return new_msg;  
}
int log_init(){
    pid_t logger = fork(); 
    if(logger == 0){

    }
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
                printf("Read operation didn't work"); 
                log_destroy(); 
                return -1; 
            }
            buffer = trim_string(buffer, 200); 
            printf("%s", buffer) ;
            free(buffer); 
            buffer = NULL; 
        }
    
    
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
   
}
