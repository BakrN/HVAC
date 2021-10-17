#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
typedef struct {
    uint32_t sequence_number; 
    char* timestamp; 
    char* message; 
} event; 

int log_init();// Fork Proecess 

void log_event(event* event); 
void log_message(char* message); 

void log_destroy(); 