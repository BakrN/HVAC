#include "connmgr.h"

//

pthread_rwlock_t connmgr_drop_sensor; // needs to be shared with data mgr
tcpsock_t *server;
dplist_t *socket_list;
struct pollfd *pollfds;
sbuffer_t *test;
log_msg *CONN_LOG_MSG;
int data_conn_pipefds[2];
int CONN_GATEWAY_FD;
typedef struct
{
    tcpsock_t *socket;
    uint16_t sensor_id;
    time_t last_timestamp;
} tcp_element;

int tcp_element_compare(void *x, void *y)
{

    if (((tcp_element *)x)->socket->sd == ((tcp_element *)y)->socket->sd)
    {
        return 0;
    }

    else if (((tcp_element *)x)->sensor_id == ((tcp_element *)y)->sensor_id)
    {
        return 0;
    }
    return -1;
}
void tcp_element_free(void **element)
{

    tcp_close(&(((tcp_element *)*element)->socket));
    free(*element);
}

void connmgr_init(void *port_number)
{
    CONN_LOG_MSG = malloc(sizeof(log_msg));
    CONN_LOG_MSG->sequence_number = 1;
    CONN_GATEWAY_FD = open("gateway.log", O_WRONLY);
    socket_list = dpl_create(NULL, tcp_element_free, &tcp_element_compare);
    pthread_rwlock_init(&connmgr_drop_sensor, NULL);
    if (!pipe2(data_conn_pipefds, O_NONBLOCK))
    {
        CONN_LOG_MSG->timestamp = time(0);
        asprintf(&CONN_LOG_MSG->message, "Datamgr Connmgr Pipe was opened successfully\n");
        log_event(CONN_GATEWAY_FD, CONN_LOG_MSG);
        free(CONN_LOG_MSG->message);
    }
    
    //./sensor_test 101 15 127.0.0.1 1234
    connmgr_listen_to_port(*(int *)port_number, &test);
    // open pipe to datagmr
}
void connmgr_listen_to_port(int port_number, sbuffer_t **buffer)
{
    if (port_number < MIN_PORT || port_number > MAX_PORT)
    {
        // log invalid port
        return;
    }
    tcpsock_t *client;
    tcp_element *oldest_sock = NULL;

    pollfds = malloc(2 * sizeof(struct pollfd));
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR)
    {
        // log failed to setup server
    }
    int conn_count = 0;
    tcp_get_sd(server, &(pollfds[0].fd));
    pollfds[0].events = POLLHUP | POLLIN;
    // add read pipe to poll
    pollfds[1].fd = data_conn_pipefds[0];
    pollfds[1].events = POLLIN;
    sensor_id_t id_to_be_dropped;
    tcp_element *temp = malloc(sizeof(tcp_element));
    temp->socket = malloc(sizeof(tcpsock_t));

    while (poll(pollfds, conn_count + 2, 20000) > 0 || conn_count)
    { // revents are cleared by poll function
#ifdef DEBUG
        printf("Current clients count: %d \n", conn_count);
#endif
        if (pollfds[0].revents & POLLIN)
        {
            if (tcp_wait_for_connection(server, &client) == TCP_NO_ERROR)
            {

                // log failed to create socket

                // successfully created new socket;
                pollfds[0].revents = 0;

                tcp_element *entry = malloc(sizeof(tcp_element));
                if (!client)
                {
                    printf("errro with client\n");
                }
                entry->socket = client;
                entry->sensor_id = 0;
                entry->last_timestamp = time(0);
                dpl_insert_at_index(socket_list, entry, 0, 0);
                if (!oldest_sock)
                {
                    oldest_sock = entry;
                }
                printf("Size of list %d\n", dpl_size(socket_list));
                pollfds = realloc(pollfds, (2 + conn_count) * sizeof(struct pollfd));
                if (tcp_get_sd(client, &(pollfds[conn_count + 2].fd)) != TCP_NO_ERROR)
                {
                    printf("Error while getting sock descriptor'n");
                }
                pollfds[conn_count + 2].events = POLLHUP | POLLIN;
                conn_count++;
#ifdef DEBUG
                printf("Successfully created socket with sd: %d \n", pollfds[conn_count + 1].fd);
#endif
                client = NULL;
                // add client socket to dp list of sockets
            }
            else
            {
#ifdef DEBUG
                printf("Failed to create socket\n");
#endif
            }
        }

        if (pollfds[1].revents & POLLIN)
        { // handle input from datamgr;

            while (read(data_conn_pipefds[0], &id_to_be_dropped, sizeof(sensor_id_t) > 0))
            {
                // drop sensors from here
                temp->sensor_id = id_to_be_dropped;
                dplist_node_t *node = dpl_get_reference_of_element(socket_list, temp);
                for (int i = 2; i < conn_count + 2; i++)
                {
                    if (pollfds[i].fd == ((tcp_element *)(node->element))->socket->sd)
                    {

                        if (node == dpl_get_reference_of_element(socket_list, oldest_sock))
                        {
                            dpl_remove_at_reference(socket_list, node, 1);
                            oldest_sock = (tcp_element *)(dpl_get_last_reference(socket_list)->element);
                        }
                        else
                        {
                            dpl_remove_at_reference(socket_list, node, 1);
                        }
                        for (int j = i; j < conn_count + 1; j++)
                        {
                            pollfds[i] = pollfds[i + 1];
                        }
                        conn_count--;
                        pollfds = realloc(pollfds, sizeof(struct pollfd) * (2 + conn_count));

                        break;
                    }
                }
            }
        }
        /*if (oldest_sock!=NULL && (time(0)-oldest_sock->last_timestamp) > TIMEOUT)
        {
            dplist_node_t *node = dpl_get_reference_of_element(socket_list, oldest_sock);
            // remove from pollfd
            int i;
            for (i = 2; i < conn_count + 2; i++)
            {
                if (pollfds[i].fd = oldest_sock->socket->sd)
                {
                    break;
                }
            }
            for (int j = i; j < conn_count + 1; j++)
            {
                pollfds[j] = pollfds[j + 1];
            }
            conn_count--;
            pollfds = realloc(pollfds, sizeof(struct pollfd) * (2 + conn_count));
            dpl_remove_at_reference(socket_list, node, 1);
            dplist_node_t* lnode =dpl_get_last_reference(socket_list); 
            if(lnode){
            oldest_sock = (tcp_element *)(lnode->element);
            }
            else{
                oldest_sock = NULL; 
            }
            printf("removed oldest socket\n"); 
          
        }*/
        
        for (int i = 2; i < conn_count + 2; i++)
        {
printf("inside loop\n"); 
            if (pollfds[i].revents & POLLHUP)
            {
#ifdef DEBUG
                printf("Client Closed Connection  \n");
#endif
                temp->socket->sd = pollfds[i].fd;
                dplist_node_t *node = dpl_get_reference_of_element(socket_list, temp);
                if (node == dpl_get_reference_of_element(socket_list, oldest_sock))
                {
                    dpl_remove_at_reference(socket_list, node, 1);
                    dplist_node_t* lnode = dpl_get_last_reference(socket_list); 
                    if(lnode){
                    oldest_sock = (tcp_element *)(lnode->element);
                    }
                    else {
                        oldest_sock = NULL; 
                    }
                }
                else
                {
                    dpl_remove_at_reference(socket_list, node, 1);
                }
                for (int j = i; j < conn_count + 1; j++)
                {
                    pollfds[j] = pollfds[j + 1];
                }
                conn_count--;
                pollfds = realloc(pollfds, sizeof(struct pollfd) * (2 + conn_count));
            }
            if (pollfds[i].revents & POLLIN)
            {

                // new data
                // receive id, value , ts

                temp->socket->sd = pollfds[i].fd;
                dplist_node_t *node = dpl_get_reference_of_element(socket_list, temp);
                tcp_element *ptr = (tcp_element *)(node->element);

                sensor_data_t *data = malloc(sizeof(sensor_data_t));
                int result, bytes;
                bytes = sizeof(data->id);
                result = tcp_receive(ptr->socket, (void *)&data->id, &bytes);
                if (bytes == 0)
                {
                    // client close connection frmo their side
#ifdef DEBUG
                    printf("Client Closing Connection ...  \n");
#endif

                    if (ptr == (tcp_element *)(dpl_get_reference_of_element(socket_list, oldest_sock)->element))
                    {

                        dpl_remove_at_reference(socket_list, node, 1);

                        dplist_node_t *lnode = dpl_get_last_reference(socket_list);
                        if (lnode)
                        {
                            oldest_sock = (tcp_element *)((dpl_get_last_reference(socket_list))->element);
                        }
                        else
                        {
                            oldest_sock = NULL;
                        }
                    }
                    else
                    {

                        dpl_remove_at_reference(socket_list, node, 1);
                    }

                    for (int j = i; j < conn_count + 1; j++)
                    {

                        pollfds[j] = pollfds[j + 1];
                    }
                    conn_count--;
                    pollfds = realloc(pollfds, sizeof(struct pollfd) * (2 + conn_count));
#ifdef DEBUG
                    printf("inside  loop  \n");
#endif

                    continue;
                }
                if (ptr->sensor_id == 0)
                {
                    ptr->sensor_id = data->id;
                }
                bytes = sizeof(data->value);
                result = tcp_receive(ptr->socket, (void *)&data->value, &bytes);
                bytes = sizeof(data->ts);
                result = tcp_receive(ptr->socket, (void *)&data->ts, &bytes);
                ptr->last_timestamp = data->ts;
                if ((result == TCP_NO_ERROR) && bytes)
                {
#ifdef DEBUG
                    printf("sensor id = %hu - temperature = %g - timestamp = %ld\n", data->id, data->value, (long int)data->ts);
#endif

                    //sbuffer_insert(*buffer, data); // don't free data
                }
                // write data to sbuffer
            }
        }

        // check last timesamp
          #ifdef DEBUG
            printf("Difference in time %ld %ld \n", time(0), oldest_sock->last_timestamp);
        #endif 
        
    }
    free(temp->socket);
    free(temp);
    connmgr_destroy();
}

void connmgr_destroy()
{

    // close only read end of pipe but will close both for testing purposes
    close(data_conn_pipefds[0]);
    close(data_conn_pipefds[1]);
    if (tcp_close(&server) != TCP_NO_ERROR)
    {
        // log failed to close server ;
    }
    free(pollfds);
    dpl_free(&socket_list, 1);
    CONN_LOG_MSG->timestamp = time(0);
    asprintf(&CONN_LOG_MSG->message, "Connmgr Destroyed Successfully");
    log_event(CONN_GATEWAY_FD, CONN_LOG_MSG);
    free(CONN_LOG_MSG->message);
    free(CONN_LOG_MSG);

    // else log successfully destroyed connmgr
}
