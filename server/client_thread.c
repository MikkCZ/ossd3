#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "client_thread.h"
#include "common/error.h"
#include "datatypes.h"
#include "common/socket.h"


void* client_thread_worker(void *data) {
  client_thread_param_t *pm = (client_thread_param_t *) data;
  client_list_t *clients = pm->clients;
  int len;

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
    mesg_send(client_socket, MESSAGE_TYPE_FATAL_ERROR, 0, "Unexpected message received", 0);
    disconnect_client(cl, clients);
    return NULL;
  }

  /* Log in the client */
  if (client_get_by_name(clients, login_msg->text) != NULL) {
    /* User is already logged in */
    mesg_send(client_socket, MESSAGE_TYPE_DISCONN, 0, "", 0);
    disconnect_client(cl, clients);
    return NULL;
  }
  cl->name = login_msg->text;
  /* Recognize client as trusted */
  /* file_login_client(cl->name); */

  /* Send user all undelivered messages */
  /* file_send_undelivered(cl); */

  message_t *msg;
  /* Wait for message from user */
  while ((len = mesg_recv(client_socket, &msg)) > 0) {
    if (msg->type != MESSAGE_TYPE_TEXT) {
      fprintf(stderr,"%sreceived message with wrong format from user%s\n",
          ERROR_PREFIX, cl->name);
      free_message(msg);
    } else {
      /* Parse the message and send it to appropriate client */
      if (send_message_to_user(client_socket, clients, msg) == -1) {
        free_message(msg);
      }
    }
  }

  if (len < 0) {
    print_error("mesg_recv");
  }

  /* Close the client socket, it is no longer needed */
  close(client_socket);

  return NULL;
}

void disconnect_client(client_item_t *cl, client_list_t *clients) {
  /* Notify the other side */
  mesg_send(cl->socket, MESSAGE_TYPE_DISCONN, 0, "", 0);

  /* Close the socket */
  close(cl->socket);

  /* Remove the client from client list */
  client_remove(clients, cl->socket);
}

int send_message_to_user(int sender, client_list_t *clients, message_t *msg) {
  char user_name[strlen(msg->text)];
  char *c = msg->text;
  int i = 0;

  /* Parse the user name */
  while (*c != ':') {
    user_name[i++] = *c;
  }
  user_name[i] = (char)0;

  if (i == 0) {
    /* Notify the sender */
    mesg_send(sender, MESSAGE_TYPE_SOFT_ERROR, msg->id, "No user name supplied", 0);
    return -1;
  }

  client_item_t *cl = client_get_by_name(clients, user_name);
  /* User not logged in, try to save it */
  if (cl == NULL) {
    /* User with this name has never been logged in */
    /* if (file_save_message(msg, user_name) == -1) { */
      /* mesg_send(sender, MESSAGE_TYPE_SOFT_ERROR, msg->id, "Invalid user name", 0); */
    print_error("Invalid user name");
      /* return -1; */
    /* } else { */
      free_message(msg);
      return -1;
    /* } */
  } else {
    /* User is logged in, send him the message */
    /* TODO: Make sure he gets it */
    /* FIXME: Message can be lost, implement checking */
    mesg_send(cl->socket, MESSAGE_TYPE_TEXT, 0 /* !!FIXME!! */, msg->text, 0);
    /* FIXME */

    free_message(msg);
  }
  return 1;
}
