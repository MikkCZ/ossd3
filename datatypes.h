/*
 * Various data types and helper functions
 */
#ifndef DATATYPES_H
#define DATATYPES_H

#include <pthread.h>

/* Linked list with client sockets */
struct client_list {
  int socket; /* Socket descriptor */
  pthread_t thread; /* Client thread descriptor */
  struct client_list *next;
};

#endif /* end of include guard: DATATYPES_H */
