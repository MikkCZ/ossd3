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
  int client_socket = pm->socket;

  /* Wait for login */
  message_t *login_msg;
  if (mesg_recv(client_socket, &login_msg) <= 0) {
    /* disconnect_client(cl, clients); */
    free(pm);
    close(client_socket);
    print_error("error receiving login packet");
    return NULL;
  }

  /* Check the packet type */
  if (login_msg->type != MESSAGE_TYPE_LOGIN) {
    print_error("wrong type of packet received");
    mesg_send(client_socket, MESSAGE_TYPE_FATAL_ERROR, 0, "Unexpected message received", 0);
    /* disconnect_client(cl, clients); */
    free(pm);
    free_message(login_msg);
    close(client_socket);
    return NULL;
  }

  printf("user '%s' connected\n", login_msg->text);

  /* Log in the client */
  if (client_get_by_name(clients, login_msg->text) != NULL) {
    /* User is already logged in */
    mesg_send(client_socket, MESSAGE_TYPE_DISCONN, 0, "", 0);
    printf("user '%s' already logged in, disconnecting\n", login_msg->text);
    /* disconnect_client(cl, clients); */
    free(pm);
    free_message(login_msg);
    close(client_socket);
    return NULL;
  }

  /* Add new client to client list */
  client_item_t *cl = client_add(pm->clients, pm->socket, &(pm->thread));
  /* Free unused memory */
  free(pm);
  /* Memory allocation failed */
  if (cl == NULL) {
    disconnect_client(cl, clients);
    free_message(login_msg);
    print_error("memory allocation error");
    return NULL;
  }
  if ((cl->name = (char *) malloc((login_msg->text_len+1)*sizeof(char))) == NULL) {
    disconnect_client(cl, clients);
    free_message(login_msg);
    print_error("memory allocation error");
    return NULL;
  }
  mesg_send(client_socket, MESSAGE_TYPE_OK, 0, "", 0);
  strcpy(cl->name, login_msg->text);
  free_message(login_msg);

  /* Recognize client as trusted */
  /* file_login_client(cl->name); */

  /* Send user all undelivered messages */
  /* file_send_undelivered(cl); */

  /* Detach thread */
  pthread_detach(cl->recv_thread);

  /* Start the sending thread */
  /*if (pthread_create(&(cl->send_thread), NULL, client_send_worker, cl) != 0) {
    disconnect_client(cl, clients);
    print_error("create sending thread failed");
    return NULL;
  }*/
  message_t *msg;
  /* Wait for message from user */
  while ((len = client_mesg_recv(cl, &msg)) > 0) {
    if (msg->type != MESSAGE_TYPE_TEXT) {
      fprintf(stderr,"%sreceived message with wrong format from user%s\n",
          ERROR_PREFIX, cl->name);
      free_message(msg);
    } else {
      /* Parse the message and send it to appropriate client */
      if (send_message_to_user(cl, clients, msg) == -1) {
        free_message(msg);
      }
    }
  }

  if (len < 0) {
    print_error("mesg_recv");
  } else if (len == 0) {
    print_error("client disconnected");
  }

  /* Stop sending thread */
  pthread_cancel(cl->send_thread);

  /* Disconnect the client */
  disconnect_client(cl, clients);
  return NULL;
}

void* client_send_worker(void *data) {
  client_item_t *cl = (client_item_t *) data;
}

void disconnect_client(client_item_t *cl, client_list_t *clients) {
  /* Notify the other side */
  client_mesg_send(cl, MESSAGE_TYPE_DISCONN, 0, "", 0);

  printf("Disconnecting client: %s\n", cl->name);

  /* Close the socket */
  close(cl->socket);

  /* Remove the client from client list */
  client_remove(clients, cl->socket);
}

int send_message_to_user(client_item_t *sender, client_list_t *clients, message_t *msg) {
  char user_name[strlen(msg->text)];
  char *c = msg->text;
  int i = 0;

  /* Parse the user name */
  while (*c != 0 && *c != ':') {
    user_name[i++] = *(c++);
  }
  user_name[i] = (char)0;

  if (i == 0) {
    /* Notify the sender */
    client_mesg_send(sender, MESSAGE_TYPE_SOFT_ERROR, msg->id, "No user name supplied", 0);
    printf("Wrong message format\n");
    return -1;
  }

  client_item_t *cl = client_get_by_name(clients, user_name);
  printf("sending message to %s\n", user_name);
  /* User not logged in, try to save it */
  if (cl == NULL) {
    /* User with this name has never been logged in */
    /* if (file_save_message(msg, user_name) == -1) { */
      /* mesg_send(sender, MESSAGE_TYPE_SOFT_ERROR, msg->id, "Invalid user name", 0); */
      fprintf(stderr, "User '%s' not found\n", user_name);
      /* return -1; */
    /* } else { */
      return -1;
    /* } */
  } else {
    /* User is logged in, send him the message */
    /* TODO: Make sure he gets it */
    /* FIXME: Message can be lost, implement checking */
    client_mesg_send(cl, MESSAGE_TYPE_TEXT, 0 /* !!FIXME!! */, c+1, 0);
    /* FIXME */

    free_message(msg);
  }
  return 1;
}
