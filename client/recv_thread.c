#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "recv_thread.h"
#include "common/error.h"
#include "datatypes.h"
#include "common/socket.h"
#include "common/datatypes.h"

/* Recv thread globals: server_socket and mesg_list */
static server_socket_t *server_socket;
static mesg_list_t *mesg_list;

void* recv_thread_worker(void *data) {
	/* Get args from the struct */
	thread_args_t *args = (thread_args_t *) data;
	server_socket = args->server_socket;
	mesg_list = args->mesg_list;
	/* Every second try to get a message from the server and parse it */
	while(1) {
		message_t *new_msg;
		/* Receive the message */
		if(server_mesg_recv(server_socket, &new_msg) <= 0) {
			print_error("error receiving packet");
			break;
		}
		/* Parse it */
		if(parse_new_msg(new_msg) < 0) {
			break;
		}
	}
	/* If and error occur, it's necesery to cancel the terminal thread - main waits for join */
	pthread_cancel(*(args->terminal_thread));
	return NULL;
}

int parse_new_msg(message_t *new_msg) {
	/* Act according to the message type */
	if(new_msg->type == MESSAGE_TYPE_DISCONN) {
		print_error("disconnect msg received from the server");
		return -1;
	} else if(new_msg->type == MESSAGE_TYPE_FATAL_ERROR) {
		print_error(new_msg->text);
		return -2;
	} else if(new_msg->type == MESSAGE_TYPE_SOFT_ERROR) { // can be continued - just info from the server
		print_error(new_msg->text);
		return MESSAGE_TYPE_SOFT_ERROR;
	} else if(new_msg->type == MESSAGE_TYPE_OK) { // confirmation from the server
		return confirm_msg_recv_by_server(new_msg);
	} else if(new_msg->type == MESSAGE_TYPE_TEXT) { // message from another user
		/* Confirm message send to the server */
		server_mesg_send(server_socket, MESSAGE_TYPE_OK, new_msg->id, "", 0);
		printf("-> %s\n", new_msg->text);
		return MESSAGE_TYPE_TEXT;
	}
	return 0;
}

int confirm_msg_recv_by_server(message_t *new_msg) {
	/* FIXME: Message can be lost, implement checking */
	/* FIXME: MSG ID */
	/* Remove the first message (which has been confirmed by the server) from the queue */
	mesg_remove_first(mesg_list);
	return 0;
}

int server_mesg_recv(server_socket_t *server_socket, message_t **msg) {
	/* Lock the socket */
	pthread_mutex_lock(&(server_socket->sock_r_lock));
	/* Receive new message from the server (without any info output) */
	int ret = mesg_recv2(server_socket->socket, msg, FALSE);
	/* Unlock the socket */
	pthread_mutex_unlock(&(server_socket->sock_r_lock));
	
	return ret;
}
