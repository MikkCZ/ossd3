#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "datatypes.h"
#include "common/error.h"
#include "common/socket.h"

/* Define the mutex */
pthread_mutex_t g_client_list_mx;

/* Message identifiers */
static uint_32 g_msg_counter = 0;
static pthread_mutex_t g_mid_mx = PTHREAD_MUTEX_INITIALIZER;

void client_free(client_item_t *cl) {
  if (cl != NULL) {
    printf("Freeing client: ");
    if (cl->name != NULL) {
      printf("%s", cl->name);
      free(cl->name);
    }
    printf("\n");

    free(cl);
  }
}

client_item_t* client_add(client_list_t *list, int socket, pthread_t *thread) {
  client_item_t *new_client;
  if ((new_client = (client_item_t *) calloc(1, sizeof(client_item_t))) ==  NULL) {
    print_error("memory allocation error");
    return NULL;
  }
  /* Initialize the struct */
  new_client->socket = socket;
  new_client->recv_thread = *thread;
  /* Initialize the client socket locks */
  pthread_mutex_init(&(new_client->sock_w_lock), NULL);
  pthread_mutex_init(&(new_client->sock_r_lock), NULL);

  pthread_mutex_lock(&g_client_list_mx);
  /* List is empty */
  if (list->end == NULL) {
    list->start = list->end = new_client;
  } else {
    list->end->next = new_client;
    list->end = new_client;
  }
  pthread_mutex_unlock(&g_client_list_mx);

  return new_client;
}

void client_remove(client_list_t *list, int socket) {
  client_item_t *p, *pp;
  for (p = pp = list->start; p != NULL; pp = p, p = p->next) {
    if (p->socket == socket) {
      pthread_mutex_lock(&g_client_list_mx);

      /* Only one item */
      if (list->start == list->end) {
        list->start = NULL;
        list->end = NULL;
      } else {
        /* Delete the start */
        pp->next = p->next;
        if (p == list->start) {
          list->start = p->next;
        }
        /* Delete the end */
        if (p == list->end) {
          list->end = pp;
        }
      }
      /* Unlock the client socket lock */
      /* pthread_mutex_unlock(&(p->sock_lock)); */
      client_free(p);
      pthread_mutex_unlock(&g_client_list_mx);
      break;
    }
  }
}

client_item_t* client_get_by_name(client_list_t *list, const char *name) {
  client_item_t *p;
  for (p = list->start; p != NULL; p = p->next) {
    if (p->name != NULL && strcmp(p->name, name) == 0) {
      return p;
    }
  }

  return NULL;
}

client_item_t* client_get_by_socket(client_list_t *list, int socket) {
  client_item_t *p;
  for (p = list->start; p != NULL; p = p->next) {
    if (p->socket == socket) {
      return p;
    }
  }

  return NULL;
}

int client_mesg_recv(client_item_t *cl, message_t **msg) {
  /* Lock the socket */
  pthread_mutex_lock(&(cl->sock_r_lock));

  int ret = mesg_recv(cl->socket, msg);

  /* Unlock the socket */
  pthread_mutex_unlock(&(cl->sock_r_lock));

  return ret;
}

int client_mesg_send(client_item_t *cl, uint_8 type, uint_32 id, 
    const char *msg, int can_fail) {
  /* Lock the socket */
  pthread_mutex_lock(&(cl->sock_w_lock));

  int ret = mesg_send(cl->socket, type, id, msg, can_fail);

  /* Unlock the socket */
  pthread_mutex_unlock(&(cl->sock_w_lock));

  return ret;
}

int conf_mesg_send(client_item_t *cl, uint_8 type, const char *msg, int can_fail) {
  /* Lock the message ids */ 
  pthread_mutex_lock(&g_mid_mx);
  uint_32 msg_id = g_msg_counter;
  g_msg_counter++;
  pthread_mutex_unlock(&g_mid_mx);

  int status;
  if ((status = client_mesg_send(cl, type, msg_id, msg, can_fail)) < 1) {
    return status;
  }

  /* Wait for confirmation */
  /*int tp, i;
  for (i = 0; i < TRY_COUNT; i++) {
    int j;
    for (j = 0; j < CHECK_COUNT; j++) {
      tp = client_mesg_peek(cl);
      /* No message in queue */
      /* if (tp == EAGAIN || tp == EWOULDBLOCK)  */

}
