/*
 */
#ifndef DATATYPES_H
#define DATATYPES_H

typedef enum __message_type_s {
  LOGIN,
  DISCONN,
  TEXT,
  FATAL_ERROR,
  SOFT_ERROR,
  OK
} message_type_t;

#endif /* end of include guard: DATATYPES_H */
