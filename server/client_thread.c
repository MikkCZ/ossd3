#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "client_thread.h"
#include "common/error.h"
#include "datatypes.h"


void* client_thread_worker(void *data) {
  client_thread_param_t *pm = (client_thread_param_t *) data;
  client_list_t *clients = pm->clients;
  int len;
  char buffer[BUFFER_SIZE];

  /* Add new client to client list */
  client_item_t *cl = client_add(pm->clients, pm->socket, &(pm->thread));
  /* Free unused memory */
  free(pm);
  int client_socket = cl->socket;
  /* Memory allocation failed */
  if (cl == NULL) {
    disconnect_client(cl, clients);
    print_error("memory allocation error");
    return NULL;
  }

  /* Wait for login */
  message_t *login_msg;
  if (mesg_recv(client_socket, &login_msg) < 0) {
    disconnect_client(cl, clients);
    print_error("error receiving login packet");
    return NULL;
  }

  /* Check the packet type */
  if (login_msg->type != MESSAGE_TYPE_LOGIN) {
    print_error("wrong type of packet received");
    disconnect_client(cl, clients);
    return NULL;
  }

  /* Log in the client */
  if (client_get_by_name(clients, login_msg->msg) != NULL) {
    /* User is already logged in */
    mesg_send(client_socket, MESSAGE_TYPE_DISCONN, 0, "", 0);
    disconnect_client(client_socket, clients);
    return NULL;
  }
  cl->name = login_msg->msg;

  /* Send user all undelivered messages */
  send_undelivered(cl);

  message_t *msg;
  /* Wait for message from user */
  while (mesg_recv(client_socket, &msg) > 0) {
    if (msg->type == MESSAGE_TYPE_TEXT) {



  if (len < 0) {
    print_error("recv");
  }

  /* Close the client socket, it is no longer needed */
  close(socket);

  return NULL;
}
