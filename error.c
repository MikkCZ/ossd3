/* String manipulation */
#include <string.h>
#include <stdio.h>
#include "error.h"


void print_error(const char *message) {
  char error_string[strlen(ERROR_PREFIX)+strlen(message)+1];
  sprintf(error_string, "%s%s", ERROR_PREFIX, message);
  perror(error_string);
}

