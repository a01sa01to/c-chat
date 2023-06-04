#pragma once

#include <netinet/in.h>
#include <pthread.h>

#include "io.h"

void *handle_send(void *arg) {
  int *sock = (int *) arg;
  char buffer[BUFSIZE];

  while (true) {
    memset(buffer, '\0', BUFSIZE);
    fgets(buffer, BUFSIZE, stdin);
    chop(buffer);
    send(*sock, encode("anonymous", buffer), BUFSIZE, 0);
    if (is_equal_str(buffer, "quit")) {
      break;
    }

    printf("> ");
  }
  printf("\n%sinfo%s sender quit\n", FONT_CYAN, FONT_RESET);
  pthread_exit(NULL);
}

void *handle_receive(void *arg) {
  int *sock = (int *) arg;
  char buffer[BUFSIZE];
  while (true) {
    memset(buffer, '\0', BUFSIZE);
    recv(*sock, buffer, BUFSIZE, 0);
    printf("\r%s%s%s\n", FONT_UNDERLINED, decode_username(buffer), FONT_RESET);
    printf(">> %s\n\n", decode_message(buffer));
    printf("\r> ");
    fflush(stdout);
    if (is_equal_str(decode_message(buffer), "quit")) {
      printf("%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      break;
    }
  }
  printf("\n%sinfo%s receiver quit\n", FONT_CYAN, FONT_RESET);
  pthread_exit(NULL);
}