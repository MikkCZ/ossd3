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
	/*TODO*/
}

void disconnect_from_server(server_socket_t *server_socket) {
	/* Notify the other side */
	server_mesg_send(server_socket, MESSAGE_TYPE_DISCONN, 0, "", 0);
	printf("Disconnecting from server\n");
	
	/* Close the socket */
	close(server_socket->socket);
}

int send_message_to_server(server_socket_t *server_socket, message_t *msg) {
	/*TODO*/
}
