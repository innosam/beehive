CC=g++
CFLAGS=-c  -g -Wall -pedantic
MKDIR_P = mkdir -p

.PHONY:	dir all clean

all: dir server client  tclient tcli 

server: obj/server.o obj/lib.o obj/master.o obj/taskManager.o obj/buffer.o
	$(CC) -g -O3 -o  server obj/server.o obj/lib.o obj/master.o obj/taskManager.o obj/buffer.o -lrt

client: obj/client.o obj/lib.o obj/mapreduce.o obj/buffer.o
	$(CC) -g -O3 -o  client obj/client.o obj/lib.o obj/mapreduce.o obj/buffer.o -lrt

tclient: obj/tclient.o obj/lib.o obj/mapreduce.o obj/buffer.o
	$(CC) -g -O3 -o  tclient obj/tclient.o obj/lib.o obj/mapreduce.o obj/buffer.o -lrt -lpthread

tcli: obj/tcli.o obj/lib.o obj/buffer.o
	$(CC) -g -O3  -o tcli obj/tcli.o obj/lib.o obj/buffer.o -lncurses -lreadline  -lrt -lpthread


obj/master.o: Master.cpp 
	$(CC) $(CFLAGS) Master.cpp -o obj/master.o

obj/mapreduce.o: MapReduce.cpp
	$(CC) $(CFLAGS) MapReduce.cpp -o obj/mapreduce.o


obj/taskManager.o: TaskManager.cpp 
	$(CC) $(CFLAGS) TaskManager.cpp -o obj/taskManager.o

obj/tclient.o: tclient.cpp 
	$(CC) $(CFLAGS) tclient.cpp -o obj/tclient.o

obj/client.o: client.cpp 
	$(CC) $(CFLAGS) client.cpp -o obj/client.o

obj/cli.o: cli.cpp 
	$(CC) $(CFLAGS) cli.cpp -o obj/cli.o

obj/tcli.o: tcli.cpp 
	$(CC) $(CFLAGS) tcli.cpp -o obj/tcli.o


obj/lib.o: lib.cpp lib.h  
	$(CC) $(CFLAGS) lib.cpp -o obj/lib.o

obj/buffer.o: Buffer.cpp Buffer.h 
	$(CC) $(CFLAGS) Buffer.cpp -o obj/buffer.o


obj/server.o: server.cpp server.h  
	$(CC) $(CFLAGS) server.cpp -o obj/server.o


dir: 
	$(MKDIR_P) obj

clean: 
	rm -r obj/ client server tcli tclient
