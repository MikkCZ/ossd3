/*
 * Thread routines for client
 */
#ifndef CLIENT_THREAD_H
#define CLIENT_THREAD_H

#include "datatypes.h"
#include "common/datatypes.h"

/* 
 * Main thread taking care of handling client requests
 */
void* client_thread_worker(void *data);

/* Remove client from list of connected clients on disconnect */
void disconnect_client(client_item_t *cl, client_list_t *clients);

/* Parse user name and send message to that user */
int send_message_to_user(client_item_t *sender, client_list_t *clients, message_t *msg);

#endif /* end of include guard: CLIENT_THREAD_H */
