#include <stdlib.h>
#include "message_queue.h"
#include "datatypes.h"
#include "common/error.h"

void queue_init(client_queue_t *q) {
  if (q != NULL) {
    q->start = q->end = NULL;
    pthread_mutex_init(&q->queue_mx, NULL);
    pthread_cond_init(&q->queue_cond, NULL);
  }
}

void queue_push(client_queue_t *queue, message_t *msg, client_item_t *sender) {
  pthread_mutex_lock(&queue->queue_mx);

  queue_item_t *item = (queue_item_t *) calloc(1, sizeof(queue_item_t));
  if (item == NULL) {
    print_error("memory allocation error");
    pthread_mutex_unlock(&queue->queue_mx);
    return;
  }
  /* Fill the queue item */
  item->msg = msg;
  item->sender = sender;

  /* Empty queue */
  if (queue_empty(queue)) {
    item->next = NULL;
    queue->start = queue->end = item;
  } else {
    item->next = NULL;
    queue->end->next = item;
  }

  /* Signal waiting thread */
  pthread_cond_signal(&queue->queue_cond);

  pthread_mutex_unlock(&queue->queue_mx);
}

queue_item_t* queue_pop(client_queue_t *queue) {
  pthread_mutex_lock(&queue->queue_mx);

  /* Wait for element */
  while (queue_empty(queue)) {
    pthread_cond_wait(&queue->queue_cond, &queue->queue_mx);
  }

  /* Pop the element */
  queue_item_t *item = queue->start;
  queue->start = item->next;

  pthread_mutex_unlock(&queue->queue_mx);

  return item;
}

int queue_empty(client_queue_t *queue) {
  return (queue->start == NULL) ? TRUE : FALSE;
}

void free_queue_item(queue_item_t *item) {
  if (item != NULL) {
    free_message(item->msg);

    free(item);
  }
}

void free_queue(client_queue_t *q) {
  if (q != NULL) {
    queue_item_t *p, *tmp;
    for (p = q->start; p != NULL;) {
      tmp = p;
      p = p->next;
      free_queue_item(tmp);
    }
  }
}
