#include "lib/tcpsock.h"
#include "config.h"
#include <poll.h>
#define TIMEOUT 120 // IN SECONDS
#define PORT 1234
// list of sensors with last timestamp
// list of client tcp sockets to store data in (might be better to auto write to sbuffer once data is received )

void connmgr_listen_to_port(int port_number); 

void connmgr_destroy(); 

void print_help(void) {
    printf("Use this program with 4 command line options: \n");
    printf("\t%-15s : a unique sensor node ID\n", "\'ID\'");
    printf("\t%-15s : node sleep time (in sec) between two measurements\n", "\'sleep time\'");
    printf("\t%-15s : TCP server IP address\n", "\'server IP\'");
    printf("\t%-15s : TCP server port number\n", "\'server port\'");
}
