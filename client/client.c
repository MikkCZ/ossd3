#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <errno.h>

/* Socket manipulation */
#include "common/socket.h"
#include "datatypes.h"

/* Thread includes */
#include "send_thread.h"
#include "recv_thread.h"
#include "terminal_thread.h"

/* Error handling */
#include "common/error.h"

#define TRUE 1
#define FALSE 0

/* Server socket */
static server_socket_t *server_socket;

/* Flag signaling that sending message can fail */
int g_fail = 0;

/* Threads */
static pthread_t send_thread;
static pthread_t recv_thread;
static pthread_t terminal_thread;

/* Linked list of messages to send with global mutex */
pthread_mutex_t mesg_mutex;
static mesg_list_t mesg_list = { NULL, NULL, &mesg_mutex };

/* Enqueue login msg */
void login(mesg_list_t* mesg_list, const char* name);

/* Clean all allocated objects */
void clean();
static int cleaned = FALSE;

/* Handler for kill signals */
void signal_handler(int signum);

/* Main block */
int main(int argc, const char *argv[])
{
	if ((server_socket = (server_socket_t *) calloc(1, sizeof(server_socket_t))) == NULL) {
		print_error("memory allocation error");
		exit(1);
	}
	/* Initialize the server socket locks */
	pthread_mutex_init(&(server_socket->sock_w_lock), NULL);
	pthread_mutex_init(&(server_socket->sock_r_lock), NULL);
	
	/* Check number of arguments */
	if (argc < 4) {
		print_error("Wrong number of args - server IP, port and login name needed.");
		exit(1);
	}

	/* Fail flag switch */
	if (argc > 4) {
		if (strcmp(argv[4], "-f") == 0) {
			g_fail = 1;
		}
	}
	
	/* Structs for network communication */
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	/* Fill in the structs */
	int status;
	struct addrinfo *res;
	if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
		fprintf(stderr, "%sgetaddrinfo failed: %s\n", ERROR_PREFIX, gai_strerror(status));
		return 1;
	}
	
	/* Loop over all results and look for valid ones */
	for (; res != NULL; res = res->ai_next) {
		
		/* Try to create socket */
		if ((server_socket->socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
			print_error("create socket");
			continue;
		}
		
		/* Try to connect the socket */
		if (connect(server_socket->socket, res->ai_addr, res->ai_addrlen) == -1) {
			close(server_socket->socket);
			print_error("connect socket");
			continue;
		}
		break;
	}
	
	/* Check the created socket */
	if (res == NULL) {
		fprintf(stderr, "%sfailed to connect\n", ERROR_PREFIX);
		return 1;
	}
	
	/* Establish the connection to the server */
	printf("Connected to server: %s\n", argv[1]);
	
	/* Catch kill signals */
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		print_error("cannot catch SIGINT signal");
	}
	if (signal(SIGTERM, signal_handler) == SIG_ERR) {
		print_error("cannot catch SIGTERM signal");
	}
	
	/* Initialize mesg mutex */
	pthread_mutex_init(&mesg_mutex, NULL);
	
	/* Add socket, mesg list and the terminal thread into to struct for threads */
	thread_args_t args;
	args.server_socket = server_socket;
	args.mesg_list = &mesg_list;
	args.terminal_thread = &terminal_thread;
	
	/* Create send thread */
	if (pthread_create(&send_thread, NULL, send_thread_worker, &args) != 0) {
		print_error("pthread_create (send)");
		clean();
		exit(1);
    }
	/* Create terminal thread */
	if (pthread_create(&terminal_thread, NULL, terminal_thread_worker, &args) != 0) {
		print_error("pthread_create (terminal)");
		clean();
		exit(1);
	}
	/* Create receive thread */
	if (pthread_create(&recv_thread, NULL, recv_thread_worker, &args) != 0) {
		print_error("pthread_create (receive)");
		clean();
		exit(1);
	}
	
	/* Send login message to the server */
	login(&mesg_list, argv[3]);
	
	/* Join terminal_thread */
	pthread_join(terminal_thread, NULL); // can be terminated by the user or by another thread (struct thread_args_t.terminal_thread)
	clean();
	return 0;
}

void login(mesg_list_t *mesg_list, const char* name) {
	message_t* login_msg;
	if ((login_msg = (message_t *) calloc(1, sizeof(message_t))) == NULL) {
		print_error("memory allocation error");
		exit(1);
	}
	
	/* Fill the message_t struct */
	login_msg->type = MESSAGE_TYPE_LOGIN;
	login_msg->id = 0;
	login_msg->text = (char *)name;
		/* Count the message length */
		int len = 0;
		while (*name != 0) {
			name++;
			len++;
		}
	login_msg->text_len = len+1;
	
	/* Enqueue the login message for sending by the send_thread */
	mesg_add(mesg_list, login_msg);
}

void clean() {
	/* Avoid double cleaning */
	if(cleaned) {
		return;
	}
	cleaned = TRUE;
	
	printf("Terminating...\n");
	/* Lock the message list */
	pthread_mutex_lock(mesg_list.mesg_mutex);
	/* Cancel threads */
	pthread_cancel(recv_thread);
	pthread_cancel(send_thread);
	pthread_cancel(terminal_thread);
	/* Send disconnect message to the server and close the socket */
	printf("Disconnecting from server...\n");
	send_disconnect_message(server_socket);
	close(server_socket->socket);
	/* Unlock the message list */
	pthread_mutex_unlock(mesg_list.mesg_mutex);
}

void signal_handler(int signum) {
	if (signum == SIGINT || signum == SIGTERM) {
		if (signum == SIGINT) {
			printf("SIGINT ");
		} else if(signum == SIGTERM) {
			printf("SIGTERM ");
		}
		printf("signal received\n");
		
		clean();
	}
}
