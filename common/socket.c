#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include "socket.h"
#include "common/error.h"

#define BUFLEN 2

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int mesg_recv(int socket, message_t **msg) {
	return mesg_recv2(socket, msg, TRUE);
}

int mesg_recv2(int socket, message_t **msg, int print_msg) {
  size_t len, received;
  char buf[BUFLEN];
  char *msg_buf = NULL;
  *msg = NULL;

  /* Just peek at the message to read the length */
  if ((len = recv(socket, (void *)buf, BUFLEN, 0)) <= 0) {
    return len;
  }

  received = 0;
  size_t msg_size;
  /* Convert first 2 bytes to number in host format */
  msg_size = ntohs(*((uint_16 *)buf));

  msg_buf = (char *) calloc(msg_size, sizeof(char));
  if (msg_buf == NULL) {
    print_error("memory allocation failed");
    return -1;
  }
  /* Copy the received buffer to the message excluding the 2 starting bytes */
  /* memcpy(msg_buf, &buf[2], received); */

  /* Keep receiving until we got the whole message */
  while (received < msg_size) {
    if ((len = recv(socket, (void *)&msg_buf[received], msg_size-received, 0)) <= 0) {
      free(msg_buf);
      return len;
    }
    received += len;
  }

  /* Create the structure, that will hold the message */
  message_t *mesg_s;
  if ((mesg_s = (message_t *) calloc(1, sizeof(message_t))) == NULL) {
    free(msg_buf);
    print_error("memory allocation failed");
    return -1;
  }

  /* Message type (1 byte) */
  mesg_s->type = *((uint_8 *) msg_buf);

  /* Message id (4 bytes) */
  mesg_s->id = ntohl(*((uint_32 *) &msg_buf[1]));

  /* Actual message */
  mesg_s->text_len = msg_size - MSG_PADDING;
  if (mesg_s->text_len > 0) {
    if ((mesg_s->text = (char *) malloc((mesg_s->text_len + 1) * sizeof(char))) == NULL) {
      free(msg);
      free(mesg_s);
      print_error("memory allocation failed");
      return -1;
    }
    memcpy(mesg_s->text, &msg_buf[MSG_PADDING], mesg_s->text_len);
    mesg_s->text[mesg_s->text_len] = (char)0;
  } else {
    mesg_s->text = NULL;
  }

  /* Update the pointer */
  *msg = mesg_s;

  free(msg_buf); msg_buf = NULL;

  /* Print to stdout */
  if (print_msg) {
    print_message(mesg_s);
  }

  /* Return success */
  return msg_size;
}

int mesg_send(int socket, uint_8 type, 
    uint_32 id, const char *msg, int can_fail) {
  if (can_fail > 0 && (double)random()/RAND_MAX < FAIL_PROBABILITY) {
    /* Fail silently giving user no feedback about the error (packet is lost) */
    return 1;
  }

  size_t m_len = strlen(msg);
  uint_16 msg_size = MSG_PADDING+m_len;
  size_t actual_length = msg_size+2; /* Length of the packet + size */
  char buffer[actual_length+2];

  uint_16 msg_size_net = htons(msg_size);
  char *c = (char *)&msg_size_net;
  /* Serialize the message size to char array */
  buffer[0] = *c; buffer[1] = *(c+1);
  
  /* Send the message type */
  buffer[2] = (char)type;

  /* Message id */
  uint_32 msg_id_net = htonl(id);
  c = (char *)&msg_id_net;
  /* Serialize the message id */
  int i;
  for (i = 0; i < 4; i++) {
    buffer[3+i] = *(c+i);
  }

  /* Actual message */
  memcpy(&buffer[2+MSG_PADDING], msg, m_len);

  int len, sent = 0;
  /* Ensure the whole message is sent */
  do {
    if ((len = send(socket, (void *)&buffer[sent], actual_length - sent, MSG_NOSIGNAL)) == -1) {
      return -1;
    }
    sent += len;
  } while(sent < actual_length);

  return 1;
}
