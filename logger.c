#include "logger.h"
#include <ctype.h> 
#include <string.h> 
FILE* log_file; 
void signal_handler(int signal){
    if (signal == SIGINT){
        #ifdef DEBUG
            printf("Logger Process terminated\n"); 
        #endif
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
    log_file = fopen("Logging.log", "wr") ; 
 
    if(mkfifo("gateway.log", 0777) == -1){
        

        if(errno != EEXIST){
            #ifdef DEBUG
            printf("failed to create log process\n"); 
            #endif
            return -1; 
            
        }
        else{
            
        }
    }
    #ifdef DEBUG
    printf("Logger Process Waiting for other end\n");
    #endif
    log_fd = open("gateway.log", O_RDONLY ) ;
    #ifdef DEBUG
    printf("Logger Process Initialized\n");
    #endif

    //the file position is advanced by that number after reading 

    char* buffer; 
        while(1){
            if(read(log_fd, buffer, 200) <= 0){
               // nothing to read
          

            }
            else{
                buffer = trim_string(buffer, 200); 
                #ifdef DEBUG
                printf("%s", buffer) ;
                #endif
                // printing to file 
                fprintf(log_file, "%s\n", buffer); 
                free(buffer); 
                buffer = NULL; 
            }
        }
        log_destroy(); 
        return 0; 
}

void log_event(int write_fd, log_msg* event){
//needs mutex here
//writes of data with size 4096 is not interleaved 

    char* test_message; 
    
    asprintf(&test_message, "%lu %ld: %s\n", event->sequence_number, event->timestamp, event->message) ; 

    write(log_fd, test_message, strlen(test_message)); 
  
    free(test_message); 
}


void log_destroy(){
    close(log_file); 
    close(log_fd); 
    exit(1); 
}


