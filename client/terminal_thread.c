#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "terminal_thread.h"
#include "common/error.h"
#include "datatypes.h"
#include "common/socket.h"

#define MAX_MSG_SIZE 160

void* terminal_thread_worker(void *data) {
	/* Get args from the struct */
	send_thread_args_t *args = (send_thread_args_t *) data;
	mesg_list_t *mesg_list = args->mesg_list;
	/* Every second try to get a message from the server and parse it */
	while(1) {
		char input[MAX_MSG_SIZE+1];
		fgets(input, MAX_MSG_SIZE, stdin);
		char *c = input;
		int len = 0;
		while (*c != 0) {
			c++;
			len++;
		}
		
		/* Ignore zero strings */
		if(len <= 1) {
			sleep(1);
			continue;
		}
		
		/* Exit/quit/logout cmds */
		if(len-1 == 4 || len-1 == 6) {
			int result = strncmp(input, "exit", len-1);
			if(result != 0) {
				result = strncmp(input, "quit", len-1);
			}
			if(result != 0) {
				result = strncmp(input, "logout", len-1);
			}
			if(result == 0) {
				break;
			}
		}
		
		/* Create and Enqueue the msg */
		message_t* new_msg;
		if ((new_msg = (message_t *) calloc(1, sizeof(message_t))) ==  NULL) {
			print_error("memory allocation error");
			break;
		}
		new_msg->type = MESSAGE_TYPE_TEXT;
		new_msg->id = 2;
		new_msg->text = (char *)input;
		new_msg->text_len = len-1;
		
		mesg_add(mesg_list, new_msg);
//		printf("enqueued\n");
		sleep(5);
	}
	return NULL;
}
