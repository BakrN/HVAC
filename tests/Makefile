CC = gcc
CFLAGS = -DDEBUG -Wall 
#DEPS = ../datamgr.h ../hashtable.h ../lib/dplist.h 
#OBJ = datamgr_test.o ../hashtable.o ../datamgr.o ../lib/dplist.o
DEPS = ../sensor_db.h ../logger.h 
OBJ = strmgr_test.o ../sensor_db.o ../logger.o

%.o : %.c %{DEPS} 
	$(CC)-c -o $@ $< $(CFLAGS)

#datamgr_test: $(OBJ) 
#	$(CC)  $(OBJ) $(CLAGS) -o datamgr_test
	 

strmgr_test: $(OBJ) 
	$(CC)  $(OBJ) $(CLAGS) -o strmgr_test 


.PHONY: clean datamgr_run_test
datamgr_run_test: 
	./datamgr_test


clean: 
	rm datamgr_test $(OBJ)
