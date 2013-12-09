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

void file_save_message(message_t *msg, const char *cl_name, const char *sender) {
}

