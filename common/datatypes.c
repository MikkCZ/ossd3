#include <stdlib.h>
#include <stdio.h>
#include "datatypes.h"

void free_message(message_t *msg) {
  if (msg != NULL) {
    /* Free the text message */
    if (msg->text != NULL) {
      free(msg->text);
    }

    /* Free the struct */
    free(msg);
  }
}

void print_message(message_t *msg) {
  printf("Msg id(%d): type = '%d', text = \"%s\"\n", msg->id, msg->type, msg->text);
}
