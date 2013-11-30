#include <stdlib.h>
#include "datatypes.h"

void free_message(message_t *msg) {
  /* Free the text message */
  if (msg->text != NULL) {
    free(msg->text);
  }

  /* Free the struct */
  free(msg);
}
