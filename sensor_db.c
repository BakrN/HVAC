#include "sensor_db.h" 

#include <string.h> 
// typedef int (*callback_t)(void *, int, char **, char **); // zero if successfull else error
/*The 2nd argument to the sqlite3_exec() callback function is the number of columns in the result. 
The 3rd argument to the sqlite3_exec() callback is an array of pointers to strings obtained as if from sqlite3_column_text(), 
one for each column. If an element of a result row is NULL then the corresponding string pointer for the sqlite3_exec()
 callback is a NULL pointer. The 4th argument to the sqlite3_exec() callback is an array of pointers to strings where each entry 
 represents the name of corresponding result column as obtained from sqlite3_column_name().*/


// extern pthread_rwlock_t sbuffer_edit_mutex; 
// extern pthread_cond_t sbuffer_element_added; 
 
//sbuffer_table_entry* strmgr_iterator;
/* Table should contain: \
id: automatically generated unique id (AUTOINCREMENT)
•sensor_id (INT)
•sensor_value (DECIMAL(4,2))
•timestamp (TIMESTAMP)*/ 


// arg = 1st arg relayed back, int = no.columns,  1st char** array of pointers to strings, one for each column, 2nd char is column names
// 
//function called for each row 
void command_callback(void* arg , int col_no, char** result, char** col_names){
    return ;
}

int create_dbtable(DBCONN* db){

    char cr[180] = "CREATE TABLE IF NOT EXISTS "; 
    strcat(cr,TO_STRING(TABLE_NAME)); 
    char cr_val[135] = " (id INTEGER PRIMARY KEY AUTOINCREMENT,sensor_id INTEGER NOT NULL,sensor_value DECIMAL(4,2) DEFAULT 0,timestamp DATETIME DEFAULT 0) ;"; 
    strcat(cr, cr_val); 
    int sq_success = sqlite3_exec(db, cr, command_callback,NULL,NULL); 
    if (sq_success!= SQLITE_OK){
    
        return -1; 
    }
 
    
    
    return 0; 
}

// THIS CODE ASSUMES THAT ONLY 1 THREAD HAS ACCESS TO THE DB 
void* strgmgr_init(void* args){
  
    strgmgr_args* ptr = (strgmgr_args*)args; 
    STRGMGR_DATA* strgmgr_data = strmgr_init_connection(ptr->clear_flag); 
    strgmgr_data->terminate_reader_thread = ptr->terminate_thread; 
    // while loop
    insert_sensor_from_sbuffer(strgmgr_data, ptr->buffer);  
    // 
    disconnect(strgmgr_data); 
    return NULL; 
}

/**
 * Make a connection to the database server
 * Create (open) a database with name DB_NAME having 1 table named TABLE_NAME  
 * \param clear_up_flag if the table existed, clear up the existing data when clear_up_flag is set to 1
 * \return the connection for success, NULL if an error occurs
 */
STRGMGR_DATA* strmgr_init_connection(char clear_up_flag){ 
    STRGMGR_DATA* strgmgr_data = malloc(sizeof(STRGMGR_DATA)); 
    strgmgr_data->DB_GATEWAY_FD = open("gateway.log", O_WRONLY); 
    strgmgr_data->DB_LOG_MSG = malloc(sizeof(log_msg)); 
    strgmgr_data->DB_LOG_MSG->sequence_number = DB_LOG_SEQUENCE_NO; 
    
    int sq_open = sqlite3_open(TO_STRING(DB_NAME), &strgmgr_data->db); // should replace this with sqlite_openv2(DB_NAME, &db,SQLITE_OPEN_NOMUTEX ,NULL)
    if(sq_open != SQLITE_OK){
        // LOG errror 
        if(sq_open == SQLITE_OK)
        disconnect(strgmgr_data->db); 
        /* 
        STRG MGR DESTROY 
        */ 
        return NULL; 
    } 
    strgmgr_data->DB_LOG_MSG->timestamp = time(0); 
    asprintf(&strgmgr_data->DB_LOG_MSG->message, "Database with name (%s) opened successfully", TO_STRING(DB_NAME)); 
    log_event(strgmgr_data->DB_GATEWAY_FD,strgmgr_data->DB_LOG_MSG); 
    free(strgmgr_data->DB_LOG_MSG->message);  
  
            // log opened database successfully
            // check if table name exists else create it; 
    create_dbtable(strgmgr_data->db);
    // log table opened successfully    
    strgmgr_data->DB_LOG_MSG->timestamp = time(0); 
    asprintf(&strgmgr_data->DB_LOG_MSG->message, "Table with name (%s) opened successully", TO_STRING(TABLE_NAME)); 
    log_event(strgmgr_data->DB_GATEWAY_FD, strgmgr_data->DB_LOG_MSG); 
        free(strgmgr_data->DB_LOG_MSG->message); 
  
    char* errDB_LOG_MSG = 0; 
    if(clear_up_flag){
        
        char sql_clear[100] = "DELETE from "; 
        strcat(sql_clear, TO_STRING(TABLE_NAME)); 
        int sq_success = sqlite3_exec(strgmgr_data->db,sql_clear, command_callback, NULL, &errDB_LOG_MSG); 
        
        if(sq_success != SQLITE_OK){
            // loggg error message
             strgmgr_data->DB_LOG_MSG->timestamp = time(0); 
            asprintf(&strgmgr_data->DB_LOG_MSG->message, "Failed cleared table");
            log_event(strgmgr_data->DB_GATEWAY_FD, strgmgr_data->DB_LOG_MSG);  
            free(strgmgr_data->DB_LOG_MSG->message); 
       
            disconnect(strgmgr_data->db); 
            
            return NULL; 
        }
        else{
            // Log successfully cleared 
             strgmgr_data->DB_LOG_MSG->timestamp = time(0); 
            asprintf(&strgmgr_data->DB_LOG_MSG->message, "Successfully cleared table"); 
            log_event(strgmgr_data->DB_GATEWAY_FD, strgmgr_data->DB_LOG_MSG); 
            free(strgmgr_data->DB_LOG_MSG->message); 
          
        } 
       
    }
   
    return strgmgr_data; 
}

/**
 * Disconnect from the database server
 * \param conn pointer to the current connection
 */
void disconnect( STRGMGR_DATA *strgmgr_data){
  
   
   
   int sq_close = sqlite3_close(strgmgr_data->db); 
    if(sq_close != SQLITE_OK){
     //   Log: failed to close db  
     strgmgr_data->DB_LOG_MSG->timestamp = time(0); 
      asprintf(&strgmgr_data->DB_LOG_MSG->message, "Failed to disconnectthe %s database", TO_STRING(DB_NAME)); 
      log_event(strgmgr_data->DB_GATEWAY_FD, strgmgr_data->DB_LOG_MSG); 
      free(strgmgr_data->DB_LOG_MSG->message); 
      free(strgmgr_data->DB_LOG_MSG); 
       return ; 
    } 
      strgmgr_data->DB_LOG_MSG->timestamp = time(0); 
      asprintf(&strgmgr_data->DB_LOG_MSG->message, "Successfully disconnected the %s database", TO_STRING(DB_NAME)); 
      log_event(strgmgr_data->DB_GATEWAY_FD, strgmgr_data->DB_LOG_MSG); 
      free(strgmgr_data->DB_LOG_MSG->message); 
    free(strgmgr_data->DB_LOG_MSG); 
    free(strgmgr_data); 
}

/**
 * Write an INSERT query to insert a single sensor measurement
 * \param conn pointer to the current connection
 * \param id the sensor id
 * \param value the measurement value
 * \param ts the measurement timestamp
 * \return zero for success, and non-zero if an error occurs
 */
int insert_sensor(STRGMGR_DATA*strmgr_data, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    char* insert_command ; 
    
    //int sq_success = sql; 
    // INSERT INTO TABLE_NAME (sensor_id, sensor_value , timestamp) VALUES(id,value,  convert(datetime, ts)
    asprintf(&insert_command, "INSERT INTO %s (sensor_id, sensor_value , timestamp) VALUES(%hu,%f,  datetime(%ld,'unixepoch'));  ", TO_STRING(TABLE_NAME), id, value, ts); 
    char* err_msg ; 
    int sq_success = sqlite3_exec(strmgr_data->db, insert_command, command_callback, NULL, &err_msg); 
    if(sq_success != SQLITE_OK) {
        // log failed DB_LOG_MSG
           strmgr_data->DB_LOG_MSG->timestamp = time(0); 
           asprintf(&strmgr_data->DB_LOG_MSG->message, "Failed to register new entry of SensorID: %hu , error msg: %s", id, err_msg)    ; 
           log_event(strmgr_data->DB_GATEWAY_FD, strmgr_data->DB_LOG_MSG);
        free(strmgr_data->DB_LOG_MSG->message); 
        return -1; 
    }
    strmgr_data->DB_LOG_MSG->timestamp = time(0); 
    asprintf(&strmgr_data->DB_LOG_MSG->message, "Successfully registered new entry of SensorID: %hu", id)    ; 
    log_event(strmgr_data->DB_GATEWAY_FD, strmgr_data->DB_LOG_MSG);
    free(strmgr_data->DB_LOG_MSG->message); 
    free(insert_command); 
    return 0; 
}

/**
 * Write an INSERT query to insert all sensor measurements available in the file 'sensor_data'
 * \param conn pointer to the current connection
 * \param sensor_data a file pointer to binary file containing sensor data
 * \return zero for success, and non-zero if an error occurs
 */
void* init_strgmgr(void* args){ 

} 

int insert_sensor_from_sbuffer(STRGMGR_DATA *strmgr_data, sbuffer_t* buffer){

    sbuffer_table_entry* entry_ptr = NULL; 
    while (!(*(strmgr_data->terminate_reader_thread)) ) {
        entry_ptr = get_next(buffer, STORE_ENTRY); 
       pthread_mutex_lock(&(buffer->sbuffer_edit_mutex)); 
        while( *(strmgr_data->terminate_reader_thread) == 0 && entry_ptr ==NULL){
            // both threads are stuck 
            printf("strgmgr sleeping\n"); 
            pthread_cond_wait(&(buffer->sbuffer_element_added), &(buffer->sbuffer_edit_mutex)); // puts thread to sleep until another thread broadcasts condition 
             entry_ptr = get_next(buffer, STORE_ENTRY); 
                   printf("strgmgr woke up\n"); 
        } 
  
 
        if(*(strmgr_data->terminate_reader_thread)&& entry_ptr == NULL){// woken up due to termination

            printf("Exited strgmgr due to termination wakeuyp\n"); 
            pthread_mutex_unlock(&(buffer->sbuffer_edit_mutex)); 
            return 0; 
        }

    
    dplist_node_t* current = entry_ptr->list->head;
    int count =  entry_ptr->tbr_strmgr; 
    // set iterator to null to have it updated when new insertion is made 
    buffer->strmgr_iterator = NULL; 
    pthread_mutex_unlock(&(buffer->sbuffer_edit_mutex)); 
    for (int i = 0 ; i < count; i++){
        sensor_data_t* data = (sensor_data_t*)current->element; 
        insert_sensor(strmgr_data, data->id, data->value,data->ts); 
        #ifdef DEBUG 
            printf("Inserted data to db: id: %hu, value: %f ts: %ld\n", data->id,data->value, data->ts ); 
            #endif
        current = current->next; 

    }

    sbuffer_update_entry(buffer, entry_ptr, STORE_ENTRY, count); 

    }

    //terminated 
    return 0; 
}
/**
 * Write a SELECT query to select all sensor measurements in the table 
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_all(DBCONN *conn, callback_t f){
    char* command; 
    asprintf(&command, "SELECT * from %s ", TO_STRING(TABLE_NAME)); 
    char* errmsg; 
    int status = sqlite3_exec(conn, command, f, NULL,&errmsg); 
    if(status != SQLITE_OK){
        free(command); 
        if(errmsg){
            free(errmsg); 
        }
        return -1; 
    }
        free(command); 
        if(errmsg){
            free(errmsg); 
        }
    return 0; 
}

/**
 * Write a SELECT query to return all sensor measurements having a temperature of 'value'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param value the value to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_by_value(DBCONN *conn, sensor_value_t value, callback_t f){
char* command; 
    asprintf(&command, "SELECT * from %s WHERE sensor_value == %f", TO_STRING(TABLE_NAME), value); 
    char* errmsg; 
    int status = sqlite3_exec(conn, command, f, NULL,&errmsg); 
    if(status != SQLITE_OK){
        free(command); 
        if(errmsg){
            free(errmsg); 
        }
        return -1; 
    }
        free(command); 
        if(errmsg){
            free(errmsg); 
        }
    return 0  ; 
}

/**
 * Write a SELECT query to return all sensor measurements of which the temperature exceeds 'value'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param value the value to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_exceed_value(DBCONN *conn, sensor_value_t value, callback_t f) {
char* command; 
    asprintf(&command, "SELECT * from %s WHERE sensor_value > %f", TO_STRING(TABLE_NAME), value); 
    char* errmsg; 
    int status = sqlite3_exec(conn, command, f, NULL,&errmsg); 
    if(status != SQLITE_OK){
        free(command); 
        if(errmsg){
            free(errmsg); 
        }
        return -1; 
    }
        free(command); 
        if(errmsg){
            free(errmsg); 
        }
    return 0  ; 
}

/**
 * Write a SELECT query to return all sensor measurements having a timestamp 'ts'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param ts the timestamp to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_by_timestamp(DBCONN *conn, sensor_ts_t ts, callback_t f){
    char* command; 
    asprintf(&command, "SELECT * from %s WHERE ts == %ld", TO_STRING(TABLE_NAME), ts); 
    char* errmsg; 
    int status = sqlite3_exec(conn, command, f, NULL,&errmsg); 
    if(status != SQLITE_OK){
        free(command); 
        // log error msg  
        
        // 
        if(errmsg){
            free(errmsg); 
        }
        return -1; 
    }
        free(command); 
        if(errmsg){
            free(errmsg); 
        }
    return 0  ; 
}

/**
 * Write a SELECT query to return all sensor measurements recorded after timestamp 'ts'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param ts the timestamp to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_after_timestamp(DBCONN *conn, sensor_ts_t ts, callback_t f){
        char* command; 
    asprintf(&command, "SELECT * from %s WHERE ts > %ld", TO_STRING(TABLE_NAME), ts); 
    char* errmsg; 
    int status = sqlite3_exec(conn, command, f, NULL,&errmsg); 
    if(status != SQLITE_OK){
        free(command); 
        // log error msg  
        
        // 
        if(errmsg){
            free(errmsg); 
        }
        return -1; 
    }
        free(command); 
        if(errmsg){
            free(errmsg); 
        }
    return 0  ; 
}

