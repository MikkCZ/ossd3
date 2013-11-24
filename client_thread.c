#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "client_thread.h"
#include "error.h"


void* client_thread_worker(void *data) {
  int* socket = (int *)data;
  int len;
  char buffer[BUFFER_SIZE];

  while ((len = recv(*socket, (void *)buffer, BUFFER_SIZE-1, 0)) > 0) {
    buffer[len] = (char)0;

    fprintf(stdout, "server received: %s\n", buffer);
  }

  if (len < 0) {
    print_error("recv");
  }

  return NULL;
}
