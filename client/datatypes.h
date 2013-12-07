/*
 * Various data types and helper functions
 */
#ifndef DATATYPES_H
#define DATATYPES_H

#include <pthread.h>
#include "common/datatypes.h"

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

#endif /* end of include guard: DATATYPES_H */
