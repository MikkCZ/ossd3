/*
 * TCP mail server
 *
 * Authors: Matej Vavrinec & Michal Stanke
 * Operating systems and networks, 2013
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C), 2013
 */
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

/* Socket manipulation */
#include "common/socket.h"
#include "datatypes.h"

/* Client thread */
#include "client_thread.h"

/* Error handling */
#include "common/error.h"

#include "file.h"

#define QUEUE_SIZE 10 /* Size of the listen queue */

/* Server socket */
static int g_server_socket;

/* Flag signaling that sending message can fail */
int g_fail = 0;

/* Linked list of client sockets and threads */
static client_list_t g_clients = { NULL, NULL };

pthread_mutex_t client_mutex;

/* Clean all allocated objects */
void clean();

/* Show usage on standard output */
void usage(const char *argv0);

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

  if (argc < 2) {
    print_error("port number is missing");
    usage(argv[0]);
    exit(1);
  } else if (strcmp(argv[1], "-h") == 0) {
    usage(argv[0]);
    exit(0);
  }

  /* Fail flag switch */
  if (argc > 2) {
    if (strcmp(argv[2], "-f") == 0) {
      g_fail = 1;
    } 
  }

  /* Structs for network communication */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  /* Fill in the structs */
  if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
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

    /* Reuse previously bound port */
    if (setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, &yes,
          sizeof(int)) == -1) {
      print_error("setsockopt");
      exit(1);
    }
    /* Bind the socket with localhost and port */
    if (bind(g_server_socket, res->ai_addr, res->ai_addrlen) == -1) {
      print_error("bind socket");
      continue;
    }

    break;
  }

  if (res == NULL) {
    fprintf(stderr, "%sfailed to bind socket\n", ERROR_PREFIX);
    return 1;
  }

  /* Free the getaddrinfo results */
  freeaddrinfo(res);

  if (listen(g_server_socket, QUEUE_SIZE) == -1) {
    print_error("listen");
    return 1;
  }

  /* Catch kill signals */
  if (signal(SIGINT, signal_handler) == SIG_ERR) {
    print_error("cannot catch SIGINT signal");
  }
  if (signal(SIGTERM, signal_handler) == SIG_ERR) {
    print_error("cannot catch SIGTERM signal");
  }

  /* Initialize client list mutex */
  pthread_mutex_init(&g_client_list_mx, NULL);
  int client_socket;
  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  char ip[INET6_ADDRSTRLEN];

  /* Initialize file handling */
  file_init();

  while ((client_socket = accept(g_server_socket, (struct sockaddr *)&client_addr, &addr_size)) != -1) {
    /* Get client IP address */
    inet_ntop(client_addr.ss_family,
        get_in_addr((struct sockaddr *)&client_addr),
        ip, sizeof(ip));
    printf("new client connected: %s\n", ip);

    client_thread_param_t *pm;
    if ((pm = (client_thread_param_t *) malloc(sizeof(client_thread_param_t))) == NULL) {
      print_error("memory allocation error");
      clean();
      exit(1);
    }
    pm->socket = client_socket;
    pm->clients = &g_clients;
    if (pthread_create(&(pm->thread), NULL, client_thread_worker, pm) != 0) {
      print_error("pthread_create");
      free(pm);
    } 

    addr_size = sizeof(client_addr);
  }
  
  /* Cleanup */
  clean();

  return 0;
}

void clean() {
  client_item_t *p = g_clients.start, *tmp;
  while (p != NULL) {
    /* stop for the client thread */
    pthread_cancel(p->recv_thread);
    pthread_cancel(p->send_thread);

    tmp = p->next;
    disconnect_client(p, &g_clients);
    p = tmp;
  }

  /* close the server socket */
  close(g_server_socket);
}

void usage(const char *argv0) {
  fprintf(stdout, "usage:  %s -h\n\t%s <portno> [-f]\n\n", argv0, argv0);
  fprintf(stdout, "\
      \t-h\t\tShow this help\n\
      \t<portno>\tNumber of port on which the server listens\n\
      \t-f\t\tSending message will fail with defined probability\n");
}
