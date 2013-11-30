/*
 * Datatypes used by client and server
 */
#ifndef COMMON_DATATYPES_H
#define COMMON_DATATYPES_H

#include <ao/os_types.h>

#define MESSAGE_TYPE_LOGIN       0
#define MESSAGE_TYPE_DISCONN     1
#define MESSAGE_TYPE_TEXT        2
#define MESSAGE_TYPE_OK          3
#define MESSAGE_TYPE_SOFT_ERROR  4
#define MESSAGE_TYPE_FATAL_ERROR 5

/* 1 byte for type and 4 bytes for id */
#define MSG_PADDING 5

/* Message struct */
typedef struct __message_s {
  uint_8 type;     /* Type of message (one of MESSAGE_TYPE_*) */
  uint_32 id;      /* Message id */
  char *text;       /* Actual message */
  size_t text_len;  /* Message length */
} message_t;

/* Free message struct */
void free_message(message_t *msg);

#endif /* end of include guard: DATATYPES_H */
