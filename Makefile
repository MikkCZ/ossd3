CC=gcc
CFLAGS=-O2 -g -Wall
LIBS=-lpthread
SERVER_OBJ=\
	client_thread.o \
	socket.o \
	error.o \
	server.o

all: server

server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o server
