/*
 * Thread routines for client
 */
#ifndef TERMINAL_THREAD_H
#define TERMINAL_THREAD_H

#include "datatypes.h"
#include "common/datatypes.h"

/* 
 * Main thread taking care of handling written messages
 */
void* terminal_thread_worker(void *data);

#endif /* end of include guard: TERMINAL_THREAD_H */
