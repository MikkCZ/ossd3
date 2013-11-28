/*
 * Various data types and helper functions
 */
#ifndef DATATYPES_H
#define DATATYPES_H

#include <pthread.h>

/* List item containing client socket, thread and name */
typedef struct __client_item_s {
  char *name; /* Client name */
  int socket; /* Socket descriptor */
  pthread_t thread; /* Client thread descriptor */
  struct __client_item_s *next;
} client_item_t;
/* Linked list with client sockets */
typedef struct __client_list_s {
  client_item_t *start;
  client_item_t *end;
} client_list_t;

/* Client socket and list of all other clients
 * Used by client thread for sending messages */
typedef struct __client_thread_param_s {
  int socket;             /* Client's socket */
  pthread_t thread;       /* Client thread descriptor */
  client_list_t *clients; /* All other clients */
} client_thread_param_t;

/* Mutex for client list */
extern pthread_mutex_t g_client_list_mx;

/* Add new client to the list */
client_item_t* client_add(client_list_t *list, int socket, pthread_t *thread);

/* Remove client from the list */
void client_remove(client_list_t *list, int socket);

/* Get client pointer from the list based on client's name */
client_item_t * client_get_by_name(client_list_t *list, const char *name);

/* Get client pointer from the list based on client's socket */
client_item_t * client_get_by_socket(client_list_t *list, int socket);

#endif /* end of include guard: DATATYPES_H */
