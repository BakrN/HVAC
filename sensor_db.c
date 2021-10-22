#include "sensor_db.h" 

#include <string.h> 
// typedef int (*callback_t)(void *, int, char **, char **); // zero if successfull else error
/*The 2nd argument to the sqlite3_exec() callback function is the number of columns in the result. 
The 3rd argument to the sqlite3_exec() callback is an array of pointers to strings obtained as if from sqlite3_column_text(), 
one for each column. If an element of a result row is NULL then the corresponding string pointer for the sqlite3_exec()
 callback is a NULL pointer. The 4th argument to the sqlite3_exec() callback is an array of pointers to strings where each entry 
 represents the name of corresponding result column as obtained from sqlite3_column_name().*/




/* Table should contain: \
id: automatically generated unique id (AUTOINCREMENT)
•sensor_id (INT)
•sensor_value (DECIMAL(4,2))
•timestamp (TIMESTAMP)*/ 


int create_table(DBCONN* db){

    char* cr = "CREATE TABLE [IF NOT EXISTS] "; 
    strcat(cr,TO_STRING(TABLE_NAME)); 
    char* cr_val = " (id INT PRIMARY KEY AUTOINCREMENT,sensor_id INT NOT NULL,sensor_value DECIMAL(4,2) DEFAULT 0,timestamp DATETIME DEFAULT 0) [WITHOUT ROWID]"; 
    strcat(cr, cr_val); 
    int sq_success = sqlite3_exec(db, cr, NULL,NULL,NULL); 
    free(cr_val); 
    free(cr); 
    if(sq_success != SQLITE_OK){
        // LOg table already exists
        return 1; 
    }
    
    return 0; 
}
/**
 * Make a connection to the database server
 * Create (open) a database with name DB_NAME having 1 table named TABLE_NAME  
 * \param clear_up_flag if the table existed, clear up the existing data when clear_up_flag is set to 1
 * \return the connection for success, NULL if an error occurs
 */
DBCONN *init_connection(char clear_up_flag){ 
    DBCONN* db; 
    int sq_open = sqlite3_open(TO_STRING(DB_NAME), &db); // should replace this with sqlite_openv2(DB_NAME, &db,SQLITE_OPEN_NOMUTEX ,NULL)
    if(sq_open != SQLITE_OK){
        // LOG errror 
        disconnect(db); 
        return NULL; 
    } 
    
            // log opened database successfully
            // check if table name exists else create it; 
    if(create_table(db)){
            // Table already exists;
        
    }
    
    char* errmsg = 0; 
    if(clear_up_flag){
        
        char* sql_clear = "DELETE from "; 
        strcat(sql_clear, TO_STRING(TABLE_NAME)); 
        int sq_success = sqlite3_exec(db,sql_clear, NULL, NULL, &errmsg); 
        free(sql_clear); 
        if(sq_success != SQLITE_OK){
            // loggg error message
            disconnect(db); 
            
            return NULL; 
        }
        else{
            // Log successfully cleared 
        } 
       
    }
    return db; 
}

/**
 * Disconnect from the database server
 * \param conn pointer to the current connection
 */
void disconnect(DBCONN *conn){
    int sq_close = sqlist3_close(TO_STRING(DB_NAME)); 
    if(sq_close != SQLITE_OK){
        // Log: failed to close db  
        return ; 
    } 
    // LOG: successfully closed db; 
}

/**
 * Write an INSERT query to insert a single sensor measurement
 * \param conn pointer to the current connection
 * \param id the sensor id
 * \param value the measurement value
 * \param ts the measurement timestamp
 * \return zero for success, and non-zero if an error occurs
 */
int insert_sensor(DBCONN *conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    char* insert_command = 0; 
    //int sq_success = sql; 
    free(insert_command); 
    return 0; 
}

/**
 * Write an INSERT query to insert all sensor measurements available in the file 'sensor_data'
 * \param conn pointer to the current connection
 * \param sensor_data a file pointer to binary file containing sensor data
 * \return zero for success, and non-zero if an error occurs
 */
int insert_sensor_from_file(DBCONN *conn, FILE *sensor_data){

}

/**
 * Write a SELECT query to select all sensor measurements in the table 
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_all(DBCONN *conn, callback_t f){

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

}

