/** 
 * \author Abubakr Nada 
 * Last Name: Nada 
 * First Name: Abubakr 
 * Student Number: r0767316   
*/
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
/**
 * @brief Data needed for creation of a logger object 
 * 
 */
typedef struct {
    pthread_mutex_t log_mutex; 
    FILE* file; 
    int r_pipefd ; // fifo or pipe fd 
    int w_pipefd ; // fifo or pipe fd 
} logger_t; 
/**
 * @brief element handled by logger 
 * 
 */
typedef struct {
    uint32_t sequence_number; 
    time_t timestamp; 
    char* message; 
 
} log_msg; 
 void signal_handler(int signal);
 char* trim_string(char* str, size_t len);

/**
 * @brief This function creates a new logger object 
 * 
 * @return logger_t* 
 */
 logger_t* log_init();
/**
 * @brief Used to pass log messages to logger. thread-safe and buffer-size is unbound 
 * 
 * @param event 
 * @param logger 
 */
 void log_event( log_msg* event, logger_t* logger);
/**
 * @brief Called to free up and clean logger memory 
 * 
 * @param logger 
 */
 void log_destroy(logger_t* logger);
 /**
  * @brief used to initiate logging process  
  * 
  * @param logger 
  * @return int 
  */
 int log_start(logger_t *logger); 
 #endif