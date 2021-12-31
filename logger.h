#ifndef LOGGER_H
#define LOGGER_H
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "config.h"
#include <signal.h>
#include <pthread.h>
#include <string.h> 
#include <stdlib.h>
typedef struct {
    pthread_mutex_t log_mutex; 
    FILE* file; 
    int r_pipefd ; // fifo or pipe fd 
    int w_pipefd ; // fifo or pipe fd 
} logger_t; 
 void signal_handler(int signal);
 char* trim_string(char* str, size_t len);
 logger_t* log_init();

 void log_event( log_msg* event, logger_t* logger);

 void log_destroy(logger_t* logger);

 #endif