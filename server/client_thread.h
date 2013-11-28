/*
 * Thread routines for client
 */
#ifndef CLIENT_THREAD_H
#define CLIENT_THREAD_H

/* 
 * Main thread taking care of handling client requests
 */
void* client_thread_worker(void *data);

/* Remove client from list of connected clients on disconnect */
void disconnect_client(int client_socket, struct client_list *clients);

#endif /* end of include guard: CLIENT_THREAD_H */
