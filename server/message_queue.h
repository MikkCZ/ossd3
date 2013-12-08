/* Queue of messages to be send */
#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <pthread.h>
#include "common/datatypes.h"

/* Queue message item */
typedef struct __queue_item_s {
  message_t *msg; /* Message */
  const char *sender; /* Name of the sender */
  struct __queue_item_s *next; /* Next queue item */
} queue_item_t;

/* Queue of messages */
typedef struct __client_queue_s {
  queue_item_t *start;
  queue_item_t *end;
  pthread_mutex_t queue_mx;
  pthread_cond_t queue_cond;
} client_queue_t;

/* Initialize queue */
void queue_init(client_queue_t *q);

/* Add message to the end of the queue */
void queue_push(client_queue_t *queue, message_t *msg, const char *sender);

/* Pop item from the start of the queue */
queue_item_t* queue_pop(client_queue_t *queue);

/* Return true when the queue is empty */
int queue_empty(client_queue_t *queue);

/* Free queue item */
void free_queue_item(queue_item_t *item);

/* Free whole queue */
void free_queue(client_queue_t *q);

#endif /* end of include guard: MESSAGE_QUEUE_H */
