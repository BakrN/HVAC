#include "connmgr.h"


void connmgr_listen_to_port(int port_number){
    if(port_number < MIN_PORT || port_number > MAX_PORT){
        // log invalid port
        return ;
    }
      tcpsock_t* server, *clinet; 
      struct pollfd * pollfds[10]; 
      if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR){
          // log failed to setup server 
      }
    
} 

void connmgr_destroy(){

}