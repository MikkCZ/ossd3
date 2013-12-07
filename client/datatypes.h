/*
 * Various data types and helper functions
 */
#ifndef DATATYPES_H
#define DATATYPES_H

#include <pthread.h>
#include "common/datatypes.h"

/* Server socket struct */
typedef struct __server_socket_s {
	int socket;
	pthread_mutex_t sock_w_lock; /* Socket write lock */
	pthread_mutex_t sock_r_lock; /* Socket read lock */
} server_socket_t;

/* List item containing message */
typedef struct __mesg_item_s {
	struct __message_s *mesg;
	struct __mesg_item_s *next;
} mesg_item_t;
/* Linked list with messages */
typedef struct __mesg_list_s {
	mesg_item_t *start;
	mesg_item_t *end;
} mesg_list_t;

/* Mutex for mesg list */
extern pthread_mutex_t g_mesg_list_mx;

/* Free the mesg structure */
void mesg_free(mesg_item_t *mesg);

/* Add new mesg to the list */
mesg_item_t* mesg_add(mesg_list_t *list, message_t *mesg);

/* Remove first mesg from the list */
void mesg_remove_first(mesg_list_t *list);

/* Send message to server */
int server_mesg_send(server_socket_t *server_socket, uint_8 type, uint_32 id, 
    const char *msg, int can_fail);

#endif /* end of include guard: DATATYPES_H */
