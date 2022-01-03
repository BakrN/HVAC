/** 
 * \author Abubakr Nada 
 * Last Name: Nada 
 * First Name: Abubakr 
 * Student Number: r0767316   
*/
#include "connmgr.h"

#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>
#include <stdio.h>
#include <stdint.h> 
#include <poll.h>
#include <malloc.h> 
#define _GNU_SOURCE

//

// NOTE SHOULD PROBABLY KEEP TRACK OF SENSOR IDS SO THAT IF THEY TRY TO CONNECT AGAIN I REFUSE THEM 


int tcp_element_compare(void *x, void *y)
{
    
    if (((tcp_element *)x)->socket->sd == ((tcp_element *)y)->socket->sd)
    {
        return 0;
    }

    return -1;
}
void tcp_element_free(void **element)
{

    if(tcp_close(&(((tcp_element *)*element)->socket))!= TCP_NO_ERROR){
       // error closing socket; 
    }

    free(*element);
    *element = NULL; 
}
void* tcp_element_copy(void *src )
{
    
    tcp_element* ptr  = malloc(sizeof(tcp_element)); 
    ptr->socket = ((tcp_element*)src)->socket; 
    ptr->last_timestamp =  ((tcp_element*)src)->last_timestamp; 
    ptr->sensor_id = ((tcp_element*)src)->sensor_id; 
    return ptr; 
}
void* connmgr_init(void *args)
{   

    
    CONNMGR_DATA* connmgr_data = malloc(sizeof(CONNMGR_DATA)); 
    conn_args* c_args  = (conn_args *)args; 

    connmgr_data->data_conn_pipefd = c_args->pipefd; 
    connmgr_data->CONN_LOG_MSG = malloc(sizeof(log_msg));
    connmgr_data->CONN_LOG_MSG->sequence_number = 1;
    connmgr_data->logger = c_args->logger; 
    connmgr_data->socket_list = dpl_create(&tcp_element_copy, &tcp_element_free, &tcp_element_compare);

    connmgr_data->buffer = (sbuffer_t*) c_args->buffer; // long is same size as sbuffer_t*
    //./sensor_test 101 15 127.0.0.1 1234
    
    connmgr_listen_to_port(c_args->port_number, connmgr_data);
    connmgr_destroy(connmgr_data); 
    return NULL; 
}
void connmgr_listen_to_port(int port_number, CONNMGR_DATA* connmgr_data)
{
    if (port_number < MIN_PORT || port_number > MAX_PORT)
    {
        // log invalid port
        
        return;
    }

 

    connmgr_data->pollfds = malloc(2 * sizeof(struct pollfd));
    if (tcp_passive_open(&connmgr_data->server, port_number) != TCP_NO_ERROR)
    {
        // log failed to setup connmgr_data->server and exit 
        return ; 
    }
    int conn_count = 0;
    tcp_get_sd(connmgr_data->server, &(connmgr_data->pollfds[0].fd));
     connmgr_data->pollfds[0].events = POLLHUP | POLLIN;
    // add read pipe to poll
    connmgr_data->pollfds[1].fd = connmgr_data->data_conn_pipefd;
    connmgr_data->pollfds[1].events = POLLIN;
    sensor_id_t id_to_be_dropped;
    tcp_element *temp = malloc(sizeof(tcp_element));
    temp->socket = malloc(sizeof(tcpsock_t));

    while (poll(connmgr_data->pollfds, conn_count + 2, 10000) > 0 && !(*(connmgr_data->buffer->terminate_threads)))
    { // revents are cleared by poll function
    #ifdef DEBUG
        //printf("Current clients count: %d \n", conn_count);
    #endif
        if (connmgr_data->pollfds[0].revents & POLLIN)
        {
            tcp_element *entry = malloc(sizeof(tcp_element));// leak somehow
            entry->socket = NULL; 
            if (tcp_wait_for_connection(connmgr_data->server, &entry->socket) == TCP_NO_ERROR)
            {

                // log failed to create socket

                // successfully created new socket;
                conn_count++; 
                
                if (!entry->socket)
                {
                    printf("errro with client\n");
                }
              
                entry->sensor_id = -1;
                entry->last_timestamp = time(NULL);
                connmgr_data->socket_list = dpl_insert_at_index(connmgr_data->socket_list, entry, 0, 0);
            
                connmgr_data->pollfds = realloc(connmgr_data->pollfds, (2 + conn_count) * sizeof(struct pollfd));

                if (tcp_get_sd(entry->socket, &(connmgr_data->pollfds[conn_count + 1].fd)) != TCP_NO_ERROR)
                {
                    printf("Error while getting sock descriptor'n");
                }
                connmgr_data->pollfds[conn_count + 1].events = POLLHUP | POLLIN;
              
#ifdef DEBUG
                printf("Successfully created socket with sd: %d \n", connmgr_data->pollfds[conn_count + 1].fd);
#endif
             
                // add client socket to dp list of sockets
            }
            else
            {
#ifdef DEBUG
                printf("Failed to create socket\n");
#endif
                    free(entry); 
            }
        }

        if (connmgr_data->pollfds[1].revents & POLLIN)
        { // handle input from datamgr;

            while (read(connmgr_data->data_conn_pipefd, &id_to_be_dropped, sizeof(sensor_id_t)) == sizeof(sensor_id_t))
            {
                // drop sensors from here

                
                #ifdef DEBUG 
                printf("SENSOR WAS DROPPED BECAUSE wasn't listening to ID %u\n", id_to_be_dropped); 
                #endif
        
                dplist_node_t* node = connmgr_data->socket_list->head; 
                while(node != NULL){
                    if(((tcp_element*)(node->element))->sensor_id == id_to_be_dropped ){
                        break; 
                    }
                    node = node->next;
     
                }

                
                  int i = 2; 
                  for ( ; i < conn_count+2; i++ ){
                    if (((tcp_element*)(node->element))->socket->sd == connmgr_data->pollfds[i].fd){
                        break; 
                    }
                  }
                  connmgr_data->socket_list = dpl_remove_at_reference(connmgr_data->socket_list, node, 1);
                    for (int j = i; j < conn_count+1; j++)
                    {
                        connmgr_data->pollfds[j] = connmgr_data->pollfds[j + 1];
                    }
                    conn_count--;
                   
                    connmgr_data->pollfds = realloc(connmgr_data->pollfds, sizeof(struct pollfd) * (2 + conn_count));
                
            }
            // send message to db to delete stuff with sensor id 
            
      
        }
              
        for (int i = 2; i < conn_count + 2; i++)
        {

           
            temp->socket->sd = connmgr_data->pollfds[i].fd;
            dplist_node_t *node = dpl_get_reference_of_element(connmgr_data->socket_list, temp);
            if (!node || !node->element){
                printf("SEGMENTATION FAULT AT: %d\n", connmgr_data->pollfds[i].fd) ; 
            }
            if((time(NULL) - ((tcp_element*)(node->element))->last_timestamp) >= TIMEOUT ||connmgr_data->pollfds[i].revents & POLLHUP){
                
                //remove node ; 
                #ifdef DEBUG
                printf("Client Node with id: %hu timed out\n", ((tcp_element*)(node->element))->sensor_id); 
                #endif
               
                    connmgr_data->socket_list = dpl_remove_at_reference(connmgr_data->socket_list, node, 1);
                    
                    for (int j = i; j < conn_count+1; j++)
                    {

                        connmgr_data->pollfds[j] = connmgr_data->pollfds[j + 1];
                    }
                    conn_count--;
                    i--;
                    connmgr_data->pollfds = realloc(connmgr_data->pollfds, sizeof(struct pollfd) * (2 + conn_count));
                  

                    continue;
            }

            if (connmgr_data->pollfds[i].revents & POLLIN)
            {

                // new data
                // receive id, value , ts
                
           
               
                tcp_element *ptr = (tcp_element *)(node->element);

                sensor_data_t *data = malloc(sizeof(sensor_data_t));
                int result, bytes;
                bytes = sizeof(data->id);
                result = tcp_receive(ptr->socket,&data->id, &bytes);
               
                if (bytes == 0) // closed connection
                {
                    // client close connection frmo their side
                    #ifdef DEBUG
                    printf("Client Closing Connection ...  \n");
                    #endif

                 
                    connmgr_data->CONN_LOG_MSG->timestamp = time(0);
                    asprintf(&(connmgr_data->CONN_LOG_MSG->message), "A sensor node with id: %d has closed the connection", ptr->sensor_id);
                    log_event( connmgr_data->CONN_LOG_MSG, connmgr_data->logger);
                    free(connmgr_data->CONN_LOG_MSG->message);

                    connmgr_data->socket_list = dpl_remove_at_reference(connmgr_data->socket_list, node, 1);
                  
                    for (int j = i; j < conn_count+1; j++)
                    {

                        connmgr_data->pollfds[j] = connmgr_data->pollfds[j + 1];
                    }
                    conn_count--;
                    i--;
                    connmgr_data->pollfds = realloc(connmgr_data->pollfds, sizeof(struct pollfd) * (2 + conn_count));

                    free(data); 
                    continue;
                }
                if (ptr->sensor_id == -1)
                {
                    ptr->sensor_id = data->id;
                       connmgr_data->CONN_LOG_MSG->timestamp = time(0);
                    asprintf(&(connmgr_data->CONN_LOG_MSG->message), "A sensor node with id: %hu has opened a new connection", data->id);
                    log_event(connmgr_data->CONN_LOG_MSG, connmgr_data->logger);
                    free(connmgr_data->CONN_LOG_MSG->message);
                }
                bytes = sizeof(data->value);
                result = tcp_receive(ptr->socket, &data->value, &bytes);
                bytes = sizeof(data->ts);
                result = tcp_receive(ptr->socket,&data->ts, &bytes);
                ptr->last_timestamp = data->ts;
                if ((result == TCP_NO_ERROR) && bytes)
                {
#ifdef DEBUG
                   // printf("sensor id = %hu - temperature = %g - timestamp = %ld\n", data->id, data->value, (long int)data->ts);
#endif

                    sbuffer_insert(connmgr_data->buffer, data); // don't free data
                }
                
            }
       
        }

        // check last timesamp
        
        
    }
    free(temp->socket);
    free(temp);

}

void connmgr_destroy(CONNMGR_DATA* connmgr_data)
{

    // close only read end of pipe but will close both for testing purposes
    close(connmgr_data->data_conn_pipefd);// read
    //close(connmgr_data->data_conn_pipefds[1]);//write
    if (tcp_close(&connmgr_data->server) != TCP_NO_ERROR)
    {
        // log failed to close connmgr_data->server ;
    }
    free(connmgr_data->pollfds);
    dpl_free(&connmgr_data->socket_list, 1); // takes care of closing sockets
    connmgr_data->CONN_LOG_MSG->timestamp = time(0);
    asprintf(&(connmgr_data->CONN_LOG_MSG->message), "Connmgr Destroyed Successfully");
    log_event( connmgr_data->CONN_LOG_MSG, connmgr_data->logger);
    free(connmgr_data->CONN_LOG_MSG->message);
    free(connmgr_data->CONN_LOG_MSG);
    *(connmgr_data->buffer->terminate_threads) =1; 
   
    sbuffer_wakeup_readerthreads(connmgr_data->buffer); 
    
    free(connmgr_data); 
    // else log successfully destroyed connmgr
}