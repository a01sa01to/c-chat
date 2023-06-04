#pragma once

#include <netinet/in.h>
#include <pthread.h>

#include "io.h"

typedef struct {
  struct sockaddr_in addr;
  int sock;
} client_t;

typedef struct {
  bool is_active;
} state_t;

state_t state = { true };

void *handle_send(void *arg) {
  client_t *client = (client_t *) arg;
  char buffer[BUFSIZE];

  // 標準入力を非同期にする
  fd_set fds;
  struct timeval tv = { 0, 10 };

  while (state.is_active) {
    // 標準入力を監視する
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    int ret = select(FD_SETSIZE, &fds, NULL, NULL, &tv);
    if (ret == -1) {
      printf("%serror%s select failed\n", FONT_RED, FONT_RESET);
      exit(EXIT_FAILURE);
    }
    else if (ret != 0 && FD_ISSET(0, &fds)) {
      memset(buffer, '\0', BUFSIZE);
      fgets(buffer, BUFSIZE, stdin);
      chop(buffer);
      send(client->sock, encode("anonymous", buffer), BUFSIZE, 0);

      if (is_equal_str(buffer, "quit")) {
        state.is_active = false;
        break;
      }
    }
  }
  pthread_exit(NULL);
}

void *handle_receive(void *arg) {
  client_t *client = (client_t *) arg;
  char buffer[BUFSIZE];
  while (state.is_active) {
    memset(buffer, '\0', BUFSIZE);
    recv(client->sock, buffer, BUFSIZE, 0);
    printf("%s%s%s\n", FONT_UNDERLINED, decode_username(buffer), FONT_RESET);
    printf(">> %s\n\n", decode_message(buffer));
    printf("\r> ");
    fflush(stdout);
    if (is_equal_str(decode_message(buffer), "quit")) {
      printf("%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      state.is_active = false;
      break;
    }
  }
  pthread_exit(NULL);
}