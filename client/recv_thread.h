/*
 * Thread routines for client
 */
#ifndef RECV_THREAD_H
#define RECV_THREAD_H

#include "datatypes.h"
#include "common/datatypes.h"

/* 
 * Main thread taking care of handling received messages
 */
void* recv_thread_worker(void *data);

/* Handle new message behaviour */
int parse_new_msg(message_t *new_msg);

/* Send a confirmation message to the server */
int confirm_msg_recv_by_server(message_t *new_msg);

/* Receive new msg from the server */
int server_mesg_recv(server_socket_t *server_socket, message_t **msg);

#endif /* end of include guard: RECV_THREAD_H */
