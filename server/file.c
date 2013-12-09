#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "file.h"
#include "common/error.h"

void file_init() {
  /* Initialize the directory path */
  strncpy(DIR_PATH, getenv("HOME"), sizeof(DIR_PATH)-1);
  strncat(DIR_PATH, "/"DIR_NAME, sizeof(DIR_PATH)-1);

  struct stat st = {0};
  /* Create the directory if it doesn't already exist */
  if (stat(DIR_PATH, &st) == -1) {
    mkdir(DIR_PATH, 0755);
  }
}

void file_login_client(const char *cl_name) {
  /* Construct path to the file */
  char path[strlen(DIR_PATH)+strlen(cl_name)+2];
  sprintf(path, "%s/%s", DIR_PATH, cl_name);

  /* If file does not exist, create it */
  if (access(path, F_OK) != 0) {
    FILE *fw = fopen(path, "w");
    if (fw == NULL) {
      print_error("error creating file for user");
    } else {
      fclose(fw);
    }
  }
}

int file_save_message(message_t *msg, const char *cl_name, const char *sender) {
  /* Construct path to the file */
  char path[strlen(DIR_PATH)+strlen(cl_name)+2];
  sprintf(path, "%s/%s", DIR_PATH, cl_name);

  /* File does not exist */
  if (access(path, F_OK) != 0) {
    return -1;
  }

  /* Open the file */
  FILE *fw = fopen(path, "ab");
  if (fw == NULL) {
    print_error("error opening file for append");
    return -2;
  }

  /* Serialize the message to the file */
  size_t s_len = strlen(sender)+1;
  fwrite(&s_len, sizeof(s_len), 1, fw);
  fwrite(msg, sizeof(message_t), 1, fw);
  fwrite(sender, s_len, 1, fw);
  fwrite(msg->text, msg->text_len+1, 1, fw);
  fclose(fw);

  free_message(msg);

  return TRUE;
}

void file_send_undelivered(client_item_t *cl) {
  /* Construct path to the file */
  char path[strlen(DIR_PATH)+strlen(cl->name)+2];
  sprintf(path, "%s/%s", DIR_PATH, cl->name);

  FILE *fr = fopen(path, "rb");
  /* File does not exist => no messages */
  if (fr == NULL) {
    print_error("file does not exist");
    return;
  }

  /* Read all messages */
  while (!feof(fr)) {
    size_t s_len;

    /* Maybe read past the end */
    if (fread(&s_len, sizeof(s_len), 1, fr) != 1) {
      continue;
    }

    message_t *msg = (message_t *) calloc(1, sizeof(message_t));
    if (msg == NULL) {
      fclose(fr);
      return;
    }
    fread(msg, sizeof(message_t), 1, fr);

    char *sender = (char *) malloc(s_len * sizeof(char));
    char *text = (char *) malloc((msg->text_len+1) * sizeof(char));
    if (sender == NULL || text == NULL) {
      free(msg);
      fclose(fr);
      return;
    }

    fread(sender, s_len, 1, fr);
    fread(text, msg->text_len+1, 1, fr);
    msg->text = text;

    /* Add the message to queue */
    queue_push(&cl->queue, msg, sender);
  }

  /* Truncate the file */
  fclose(fr);
  truncate(path, 0);

  return;
}
