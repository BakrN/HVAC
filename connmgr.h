#pragma once
#include "lib/tcpsock.h"
#include "config.h"
#include "sbuffer.h"
#include "logger.h"
#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <pthread.h>
#ifndef TIMEOUT
#define TIMEOUT 20 // IN SECONDS
#endif
// list of sensors with last timestamp
// list of client tcp sockets to store data in (might be better to auto write to sbuffer once data is received )
typedef struct
{
    tcpsock_t *socket;
    int sensor_id;
    time_t last_timestamp;
} tcp_element;
typedef struct {
    tcpsock_t *server; // server
    dplist_t *socket_list; // socket_list
    struct pollfd *pollfds; // polling field descriptors
    log_msg *CONN_LOG_MSG; // CONN_LOG_MSG withe sequence no. defined 
    int data_conn_pipefd; // pipe field descroptors
    int CONN_GATEWAY_FD; // logging field desc
    sbuffer_t* buffer; // buffer where data should be placed
    char* terminate_reader_threads; 
    void* retval ;
} CONNMGR_DATA ; 
void* connmgr_init(void *args); 

void connmgr_listen_to_port(int port_number, CONNMGR_DATA* connmgr_data); 

void connmgr_destroy(CONNMGR_DATA* connmgr_data); 

 