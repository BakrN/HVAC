#include <logger.h>
int log_init(){
    if(mkfifo("filanme.log", 0777) == -1){
        if(errno != EEXIST){
            printf("failed to create log process\n"); 
            return -1; 
        }
    }
    int fd = open("filename.log", O_RDONLY | O_NONBLOCK ) ;
}

void log_event(event* event); 
void log_message(char* message); 

void log_destroy(); 