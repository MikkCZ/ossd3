#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "send_thread.h"
#include "common/error.h"
#include "datatypes.h"
#include "common/socket.h"


void* client_thread_worker(void *data) {
	/* Get args from the struct */
	send_thread_args_t *args = (send_thread_args_t *) data;
	server_socket_t *server_socket = args->server_socket;
	mesg_list_t *mesg_list = args->mesg_list;
	/* Every second try to send the first message from the queue */
	while(TRUE) {
		sleep(1);
		send_first_mesg_from_list(server_socket, mesg_list);
	}
}

void disconnect_from_server(server_socket_t *server_socket) {
	/* Notify the other side */
	server_mesg_send(server_socket, MESSAGE_TYPE_DISCONN, 0, "", 0);
	printf("Disconnecting from server\n");
	
	/* Close the socket */
	close(server_socket->socket);
}

int send_message_to_server(server_socket_t *server_socket, message_t *msg) {
	/* FIXME: Message can be lost, implement checking */
	/* FIXME: MSG ID + MSG failure */
    int ret = server_mesg_send(server_socket, msg->type, 0 /* !!FIXME!! */, (msg->text)+1, 0);
    /* FIXME */
	
	return ret;
}

int send_first_mesg_from_list(server_socket_t *server_socket, mesg_list_t *mesg_list) {
	/* Send the first message from the queue */
	pthread_mutex_lock(mesg_list->mesg_mutex);
	int ret = send_message_to_server(server_socket, mesg_list->start);
	pthread_mutex_unlock(mesg_list->mesg_mutex);
	return ret;
}
