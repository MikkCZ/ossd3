/* 
 * File operating functions
 */
#ifndef FILE_H
#define FILE_H

#include "common/datatypes.h"
#include "datatypes.h"

#define DIR_NAME "ossd3"
static char DIR_PATH[255];

/* Initialize the constants */
void file_init();

/* Create file with saved conversations for this client */
void file_login_client(const char *cl_name);

/* Send client all undelivered messages */
void file_send_undelivered(client_item_t *cl);

/* Save the message to file for later pickup */
int file_save_message(message_t *msg, const char *cl_name, const char *sender);

#endif /* end of include guard: FILE_H */
