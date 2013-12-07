/*
 * Various data types and helper functions
 */
#ifndef DATATYPES_H
#define DATATYPES_H

#include <pthread.h>
#include "common/datatypes.h"

/* Time between the checks in useconds of the incoming message queue */
#define CHECK_INT 100000 /* 100 miliseconds */
/* Number of check intervals before sending the message again */
#define CHECK_COUNT 10 /* 1 try per second */
/* Number of overall tries before returning an error */
#define TRY_COUNT 10

/* Queue message item */
typedef struct __cl_queue_item_s {
  message_t *msg; /* Message */
} cl_queue_item_t;

/* Queue of messages */
typedef struct __client_queue_s {
  cl_queue_item_t *start;
  cl_queue_item_t *end;
  pthread_mutex_t queue_mx;
  pthread_cond_t queue_cond;
} client_queue_s;
  
/* List item containing client socket, thread and name */
typedef struct __client_item_s {
  char *name; /* Client name */
  int socket; /* Socket descriptor */
  pthread_mutex_t sock_w_lock; /* Socket write lock */
  pthread_mutex_t sock_r_lock; /* Socket read lock */
  pthread_t recv_thread; /* Receiving thread descriptor */
  pthread_t send_thread; /* Sending thread descriptor */
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

/* Free the client structure */
void client_free(client_item_t *cl);

/* Add new client to the list */
client_item_t* client_add(client_list_t *list, int socket, pthread_t *thread);

/* Remove client from the list */
void client_remove(client_list_t *list, int socket);

/* Get client pointer from the list based on client's name */
client_item_t * client_get_by_name(client_list_t *list, const char *name);

/* Get client pointer from the list based on client's socket */
client_item_t * client_get_by_socket(client_list_t *list, int socket);

/* Receive whole message from client */
int client_mesg_recv(client_item_t *cl, message_t **msg);

/* Send message to client */
int client_mesg_send(client_item_t *cl, uint_8 type, uint_32 id, 
    const char *msg, int can_fail);

/* Peek at the next message and return it's type */
int client_mesg_peek(client_item_t *cl);

/* Keep sending message until other side sends confirmation */
/* TODO: implement it! */
int conf_mesg_send(client_item_t *cl, uint_8 type,
    const char *msg, int can_fail);

#endif /* end of include guard: DATATYPES_H */
