/*
 * Datatypes used by client and server
 */
#ifndef COMMON_DATATYPES_H
#define COMMON_DATATYPES_H

#define MESSAGE_TYPE_LOGIN       1
#define MESSAGE_TYPE_DISCONN     2
#define MESSAGE_TYPE_TEXT        3
#define MESSAGE_TYPE_OK          4
#define MESSAGE_TYPE_SOFT_ERROR  5
#define MESSAGE_TYPE_FATAL_ERROR 6

/* Boolean */
#define TRUE 1
#define FALSE 0

/* 1 byte for type and 4 bytes for id */
#define MSG_PADDING 5

typedef unsigned char uint_8;
typedef unsigned short uint_16;
typedef unsigned int uint_32;

/* Message struct */
typedef struct __message_s {
  uint_8 type;     /* Type of message (one of MESSAGE_TYPE_*) */
  uint_32 id;      /* Message id */
  char *text;       /* Actual message */
  size_t text_len;  /* Message length */
} message_t;

/* Free message struct */
void free_message(message_t *msg);

/* Print the contents of message for debugging purposes */
void print_message(message_t *msg);

#endif /* end of include guard: DATATYPES_H */
