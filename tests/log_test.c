
#include "../logger.h"
#include <string.h>
void main(void){
    pid_t logger = fork(); 
    if(logger == 0){
        log_init(); 
    } 
    
    log_msg* msg = malloc(sizeof(log_msg)); 
    
    msg->sequence_number = 0; 
    msg->timestamp = 0; 
    int fd = open("filename.log", O_WRONLY); 
    int i ; 
    //while(1){
    char* buffer[255]; 
    while (fgets(buffer, 255, stdin) != NULL){

    //if(i == -1){
     ///   break; 
    //}  a
    asprintf(&msg->message, "%s\n", buffer);
    msg->sequence_number++;
    msg->timestamp +=2 ;
     
    log_event(fd, msg);
    free(msg->message); 
    }
    //}
    free(msg); 
    log_destroy(); 
}