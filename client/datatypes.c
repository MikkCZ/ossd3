#include <string.h>
#include <stdlib.h>
#include "datatypes.h"
#include "common/error.h"
#include "common/socket.h"

void mesg_free(mesg_item_t *mesg) {
	if (mesg != NULL) {
		if (mesg->mesg != NULL) {
//			free_message(mesg->mesg);
		}
		free(mesg);
	}
}

mesg_item_t* mesg_add(mesg_list_t *list, message_t *mesg) {
	mesg_item_t *new_mesg;
	if ((new_mesg = (mesg_item_t *) calloc(1, sizeof(mesg_item_t))) ==	NULL) {
		print_error("memory allocation error");
		return NULL;
	}
	/* Initialize the struct */
	new_mesg->mesg = mesg;
	
	pthread_mutex_lock(list->mesg_mutex);
	/* List is empty */
	if (list->end == NULL) {
		list->start = list->end = new_mesg;
	} else {
		list->end->next = new_mesg;
		list->end = new_mesg;
	}
	pthread_mutex_unlock(list->mesg_mutex);
	
	return new_mesg;
}

void mesg_remove_first(mesg_list_t *list) {
	mesg_item_t *first;
	
	pthread_mutex_lock(list->mesg_mutex);
	/* Only one item */
	if (list->start == list->end) {
		mesg_free(list->start);
		list->start = NULL;
		list->end = NULL;
	} else {
	/* Delete the first mesg */
		first = list->start;
		list->start = first->next;
		mesg_free(first);
	}
	pthread_mutex_unlock(list->mesg_mutex);
}

int server_mesg_send(server_socket_t *server_socket, uint_8 type, uint_32 id, 
    const char *msg, int can_fail) {
	/* Lock the socket */
	pthread_mutex_lock(&(server_socket->sock_w_lock));
	
	int ret = mesg_send(server_socket->socket, type, id, msg, can_fail);
	
	/* Unlock the socket */
	pthread_mutex_unlock(&(server_socket->sock_w_lock));
	
	return ret;
}
