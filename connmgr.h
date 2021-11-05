#pragma once
#include "lib/tcpsock.h"
#include "config.h"
#include "sbuffer.h"
#include "logger.h"
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>

#include <poll.h>
#include <malloc.h> 

#include <pthread.h>
#define TIMEOUT 6 // IN SECONDS
#define PORT 1234
// list of sensors with last timestamp
// list of client tcp sockets to store data in (might be better to auto write to sbuffer once data is received )

void connmgr_init(void *port_number); 

void connmgr_listen_to_port(int port_number, sbuffer_t** buffer); 

void connmgr_destroy(); 

 