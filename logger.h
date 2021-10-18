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

int log_init();// Fork Process 

void log_event(int fd ,event* event); // write to fifo

void log_destroy(); 
