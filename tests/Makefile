CC = gcc
CFLAGS = -DDEBUG -Wall -I../ -g 
LIBS = -lsqlite3 -pthread 
#DEPS = ../datamgr.h ../hashtable.h ../lib/dplist.h 
#OBJ = datamgr_test.o ../hashtable.o ../datamgr.o ../lib/dplist.o
#DEPS = ../sensor_db.h ../logger.h 
#OBJ = strgmgr_test.o ../sensor_db.o ../logger.o

DEPS = ../connmgr.h ../lib/tcpsock.h ../lib/dplist.h ../sbuffer.h ../hashtable.h ../logger.h
OBJ = connmgr_test.o ../connmgr.o ../lib/tcpsock.o ../lib/dplist.o ../sbuffer.o ../hashtable.o ../logger.o

%.o : %.c %{DEPS} 
	$(CC)-c -o $@ $< $(CFLAGS)

#datamgr_test: $(OBJ) 
#	$(CC)  $(OBJ) $(CLAGS) -o datamgr_test
	 

connmgr_test: $(OBJ) 
	$(CC)  $(OBJ) $(CLAGS) $(LIBS) -o connmgrtest 
	


#.PHONY: clean 

#clean: 
#	rm  gateway.log connmgrtest $(OBJ)
	
