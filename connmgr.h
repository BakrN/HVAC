/** 
 * \author Abubakr Nada 
 * Last Name: Nada 
 * First Name: Abubakr 
 * Student Number: r0767316   
*/
#pragma once
#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include "lib/tcpsock.h"
#include "config.h"
#include "sbuffer.h"
#include "logger.h"

#include <pthread.h>
#ifndef TIMEOUT
#define TIMEOUT 20 // IN SECONDS
#endif

/**
 * @brief type of element stored in the connection manager socket list   
 * 
 */
typedef struct
{
    tcpsock_t *socket;
    int sensor_id;
    time_t last_timestamp;
} tcp_element;
/**
 * @brief Connection manager data struct  
 * 
 */
typedef struct {
    tcpsock_t *server; // server
    dplist_t *socket_list; // socket_list
    struct pollfd *pollfds; // polling field descriptors
    log_msg CONN_LOG_MSG; // CONN_LOG_MSG withe sequence no. defined 
    int data_conn_pipefd; // pipe field descroptors
    sbuffer_t* buffer; // buffer where data should be placed
    logger_t* logger; // logger ptr 
} CONNMGR_DATA ; 

/**
 * @brief This function starts the connection manager and takes the buffer and port number as arguments as  pointer
 * 
 * @param args 
 * @return void* 
 */
void* connmgr_init(void *args); 

/**
 * @brief start listening process  
 * 
 * @param port_number 
 * @param connmgr_data 
 */
void connmgr_listen_to_port(int port_number, CONNMGR_DATA* connmgr_data); 
/**
 * @brief Stop listening process, close connections and free up memory  
 * 
 * @param connmgr_data 
 */
void connmgr_destroy(CONNMGR_DATA* connmgr_data); 

 