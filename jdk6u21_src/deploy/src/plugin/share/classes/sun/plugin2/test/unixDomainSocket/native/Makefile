
#CC=gcc
#CFLAGS=-g

#LIBS=-lsocket
LIBS=

all: socket1clt socket1srvSingleThread socket1srvMultiThread

socket1srvSingleThread: socket1srvSingleThread.o socket1common.o
	$(CC) -o $@ $^ $(LIBS)

socket1srvMultiThread: socket1srvMultiThread.o socket1common.o
	$(CC) -o $@ $^ $(LIBS)

socket1clt: socket1clt.o socket1common.o
	$(CC) -o $@ $^ $(LIBS)

clean:
	rm -f socket1clt socket1srvSingleThread socket1srvMultiThread *.o
