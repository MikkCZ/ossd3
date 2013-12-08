CC=gcc
CFLAGS=-Wall -I. -pthread -ggdb -g -O0
SERVER_OBJ=\
	server/message_queue.o \
	server/client_thread.o \
	server/server.o \
	server/datatypes.o

COMMON_OBJ=\
	common/datatypes.o \
	common/error.o \
	common/socket.o

CLIENT_OBJ=\
	client/send_thread.o \
	client/recv_thread.o \
	client/terminal_thread.o \
	client/client.o \
	client/datatypes.o

BIN=bin

all: server client

server: $(BIN)/server
client: $(BIN)/client

$(BIN)/server: $(SERVER_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) $(SERVER_CFLAGS) -o $@ $^

$(BIN)/client: $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f server/*.o client/*.o common/*.o $(BIN)/*
