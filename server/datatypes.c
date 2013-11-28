#include <string.h>
#include "datatypes.h"
#include "common/error.h"

/* Define the mutex */
pthread_mutex_t g_client_list_mx;

client_item_t* client_add(client_list_t *list, int socket, pthread_t *thread) {
  client_item_t *new_client;
  if ((new_client = (client_item_t *) calloc(1, sizeof(client_item_t))) ==  NULL) {
    print_error("memory allocation error");
    return;
  }
  /* Initialize the struct */
  new_client->socket = socket;
  new_client->thread = *thread;

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
      /* Delete the start */
      pp->next = p->next;
      if (p == list->start) {
        list->start = p->next;
      }
      /* Delete the end */
      if (p == list->end) {
        list->end = pp;
      }
      pthread_mutex_unlock(&g_client_list_mx);
      break;
    }
  }
}

client_item_t* client_get_by_name(client_list_t *list, const char *name) {
  client_item_t *p;
  for (p = list->start; p != NULL; p = p->next) {
    if (strcmp(p->name, name) == 0) {
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

