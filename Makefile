CC=gcc
CFLAGS=-O2 -g -Wall -Icommon
SERVER_OBJ=\
	server/client_thread.o \
	server/server.o
COMMON_OBJ=\
	common/error.o \
	common/socket.o
CLIENT_OBJ=
BIN=bin
$(SERVER_OBJ): SPECIFIC_CFLAGS :=-pthread

all: server client

server: $(BIN)/server
client: $(BIN)/client

$(BIN)/server: $(SERVER_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) $(SERVER_CFLAGS) -o $@ $^ -pthread

$(BIN)/client: $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(SPECIFIC_CFLAGS) -c -o $@ $^

clean:
	rm -f server/*.o client/*.o common/*.o $(BIN)/*
