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

/* Error handling */
#include "common/error.h"

#define TRUE 1
#define FALSE 0

/* Server socket */
static int g_server_socket;

/* Linked list of messages to send */
static mesg_list_t messages = { NULL, NULL };

pthread_mutex_t mesg_mutex;
/* Clean all allocated objects */
void clean();

/* Handler for kill signals */
void signal_handler(int signum) {
	if (signum == SIGINT || signum == SIGTERM) {
		printf("signal received\n");
		clean();
	}
}

/* Main block */
int main(int argc, const char *argv[])
{
	struct addrinfo hints;
	struct addrinfo *res;
	int status;
	
	/* Check number of arguments */
	if (argc < 4) {
		print_error("Wrong number of args - server IP, port and login name needed.");
		exit(1);
	}
	
	/* Structs for network communication */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	/* Fill in the structs */
	if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
		fprintf(stderr, "%sgetaddrinfo failed: %s\n", ERROR_PREFIX, gai_strerror(status));
		return 1;
	}
	
	int yes = 1;
	/* Loop over all results and look for valid ones */
	for (; res != NULL; res = res->ai_next) {
		
		/* Try to create socket */
		if ((g_server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
			print_error("create socket");
			continue;
		}
		
		if (connect(g_server_socket, res->ai_addr, res->ai_addrlen) == -1) {
			close(g_server_socket);
			print_error("connect socket");
			continue;
		}
		break;
	}
	
	/* Check socket */
	if (res == NULL) {
		fprintf(stderr, "%sfailed to connect\n", ERROR_PREFIX);
		return 1;
	}
	
	/* Connect to server */
	char ip[INET6_ADDRSTRLEN];
	inet_ntop(res->ai_family,
		get_in_addr((struct sockaddr *)&res->ai_addr),
		ip, sizeof (ip));
	printf("connecting to server: %s\n", ip);
	
	/* Catch kill signals */
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		print_error("cannot catch SIGINT signal");
	}
	if (signal(SIGTERM, signal_handler) == SIG_ERR) {
		print_error("cannot catch SIGTERM signal");
	}
	
	/* Initialize mesg mutex */
	pthread_mutex_init(&g_mesg_list_mx, NULL);
	
	// TODO
	/* create threads for sending (1) and receiving (1)
	 * send login message (enqueue it)
	 * wait for login response (and interpret it)
	 * 		error - print, close and clean socket and all structs
	 * 		OK - create thread for reading the input
	 */
	
	return 0;
}

void clean() {
	/*send disconnect message*/
	/*cancel threads*/
	/*close the socket*/
}