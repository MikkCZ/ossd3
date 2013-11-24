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
#include "socket.h"
#include "datatypes.h"

/* Client thread */
#include "client_thread.h"

#include "error.h"

#define TRUE 1
#define FALSE 0
#define QUEUE_SIZE 10

/* Linked list of client sockets and threads */
static struct client_list *g_clients = NULL;

/* Server socket */
static int g_server_socket;

/* Handler for kill signals */
void signal_handler(int signum) {
  if (signum == SIGINT || signum == SIGTERM) {
    printf("signal received\n");
    struct client_list *p = g_clients;
    while (p != NULL) {
      /* just close the socket */
      close(p->socket);

      /* stop for the client thread */
      pthread_cancel(p->thread);
      /* pthread_join(p->thread, NULL); */

      p = p->next;

      free(p);
    }

    /* close the server socket */
    close(g_server_socket);
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
    exit(1);
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
      /* perror("socket"); */
      print_error("create socket");
      continue;
    }

    if (setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, &yes,
          sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    /* Bind the socket with localhost and port */
    if (bind(g_server_socket, res->ai_addr, res->ai_addrlen) == -1) {
      /* perror("bind"); */
      print_error("bind socket");
      continue;
    }

    break;
  }

  if (res == NULL) {
    fprintf(stderr, "%sfailed to bind socket\n", ERROR_PREFIX);
    return 1;
  }

  if (listen(g_server_socket, QUEUE_SIZE) == -1) {
    /* perror("listen"); */
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

  int client_socket;
  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  char ip[INET6_ADDRSTRLEN];
  while ((client_socket = accept(g_server_socket, (struct sockaddr *)&client_addr, &addr_size)) != -1) {
    inet_ntop(client_addr.ss_family,
        get_in_addr((struct sockaddr *)&client_addr),
        ip, sizeof(ip));
    printf("new client: %s\n", ip);
    struct client_list *c;
    if ((c = (struct client_list *) calloc(1, sizeof(struct client_list))) == NULL) {
      print_error("memory allocation error");
      signal_handler(SIGINT);
      exit(1);
    }

    if (pthread_create(&(c->thread), NULL, client_thread_worker, &client_socket) != 0) {
      print_error("pthread_create");
      free(c);
    } else {
      c->socket = client_socket;
      /* Insert new client to the begining of the client list */
      c->next = (g_clients == NULL) ? NULL : g_clients->next;
      g_clients = c;
    }

    addr_size = sizeof(client_addr);
  }
  
  perror("accept");

  signal_handler(SIGINT);

  return 0;
}
