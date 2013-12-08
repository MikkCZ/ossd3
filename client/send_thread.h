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
void send_disconnect_message(server_socket_t *server_socket);

/* Send a message to the server */
int send_message_to_server(server_socket_t *server_socket, message_t *msg);

/* Send the first message from the list to the server */
int send_first_mesg_from_list(server_socket_t *server_socket, mesg_list_t *mesg_list);

#endif /* end of include guard: SEND_THREAD_H */
