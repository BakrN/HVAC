/**
 * \author Luc Vandeurzen
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "lib/tcpsock.h"
#define SENSOR_COUNT 200
#define LOOPS 20
// conditional compilation option to control the number of measurements this sensor node wil generate
#if (LOOPS > 1)
#define UPDATE(i) (i--)
#else
#define LOOPS 1
#define UPDATE(i) (void)0 //create infinit loop
#endif

// conditional compilation option to log all sensor data to a text file
#ifdef LOG_SENSOR_DATA

#define LOG_FILE	"sensor_log"

#define LOG_OPEN()						\
    FILE *fp_log; 						\
    do { 							\
      fp_log = fopen(LOG_FILE, "w");				\
      if ((fp_log)==NULL) { 					\
    printf("%s\n","couldn't create log file"); 		\
    exit(EXIT_FAILURE); 					\
      }								\
    } while(0)

#define LOG_PRINTF(sensor_id,temperature,timestamp)							\
      do { 												\
    fprintf(fp_log, "%" PRIu16 " %g %ld\n", (sensor_id), (temperature), (long int)(timestamp));	\
    fflush(fp_log);											\
      } while(0)

#define LOG_CLOSE()	fclose(fp_log);

#else
#define LOG_OPEN(...) (void)0
#define LOG_PRINTF(...) (void)0
#define LOG_CLOSE(...) (void)0
#endif

#define INITIAL_TEMPERATURE    20
#define TEMP_DEV        5    // max afwijking vorige temperatuur in 0.1 celsius


void print_help(void);

/**
 * For starting the sensor node 4 command line arguments are needed. These should be given in the order below
 * and can then be used through the argv[] variable
 *
 * argv[1] = sensor ID
 * argv[2] = sleep time
 * argv[3] = server IP
 * argv[4] = server port
 */

int main(int argc, char *argv[]) {
    sensor_data_t data[SENSOR_COUNT];
    int server_port = 1234;
    char server_ip[] = "127.0.0.1";
    tcpsock_t *client[SENSOR_COUNT];
    int i, bytes, sleep_time;
    sleep_time = 1; 
    int initial_temp; 
    LOG_OPEN();

 
 

    srand48(time(NULL));

    // open TCP connection to the server; server is listening to SERVER_IP and PORT
    for(int j = 0 ; j < SENSOR_COUNT; j++) {if (j == 31) continue; if(tcp_active_open(&client[j], server_port, server_ip)!=TCP_NO_ERROR) exit(EXIT_FAILURE) ;}

   
    for(int k = 0 ; k < 20  ; k++) {
        for (int j= 0 ; j < SENSOR_COUNT ; j++){
          if (j == 31) continue; 
        data[j].value = data[j].value + TEMP_DEV * ((drand48() - 0.5) / 10);
        time(&data[j].ts);
        // send data to server in this order (!!): <sensor_id><temperature><timestamp>
        // remark: don't send as a struct!
        data[j].id = j; 
        bytes = sizeof(data[j].id);
        if (tcp_send(client[j], (void *) &data[j].id, &bytes) != TCP_NO_ERROR) ;
        bytes = sizeof(data[j].value);
        if (tcp_send(client[j], (void *) &data[j].value, &bytes) != TCP_NO_ERROR) ;
        bytes = sizeof(data[j].ts);
        if (tcp_send(client[j], (void *) &data[j].ts, &bytes) != TCP_NO_ERROR);
        LOG_PRINTF(data[j].id, data[j].value, data[j].ts);
       
    }
        sleep(sleep_time);
   
    
    }

    for (int j = 0 ; j < SENSOR_COUNT; j++) {if(j==31) continue;  if (tcp_close(&client[j]) != TCP_NO_ERROR) ;}

    LOG_CLOSE();

printf("Success CLOSING CLINET\n");
    exit(EXIT_SUCCESS);
}

/**
 * Helper method to print a message on how to use this application
 */
void print_help(void) {
    printf("Use this program with 4 command line options: \n");
    printf("\t%-15s : a unique sensor node ID\n", "\'ID\'");
    printf("\t%-15s : node sleep time (in sec) between two measurements\n", "\'sleep time\'");
    printf("\t%-15s : TCP server IP address\n", "\'server IP\'");
    printf("\t%-15s : TCP server port number\n", "\'server port\'");
    printf("\t%-15s : Inital sensor temperature 'init temp \'");
}