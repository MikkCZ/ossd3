/*
 * Thread routines for client
 */
#ifndef SEND_THREAD_H
#define SEND_THREAD_H

#include "datatypes.h"
#include "common/datatypes.h"

/* 
 * Main thread taking care of handling send requests
 */
void* send_thread_worker(void *data);

/* Disconnect from the server */
void disconnect_from_server(server_socket_t *server_socket);

/* Send a message to the server */
int send_message_to_server(server_socket_t *server_socket, message_t *msg);

#endif /* end of include guard: SEND_THREAD_H */
