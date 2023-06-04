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
  printf("handle_send start\n");

  while (state.is_active) {
    // 標準入力を監視する
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    int ret = select(FD_SETSIZE, &fds, NULL, NULL, &tv);
    if (ret == -1) {
      printf("%serror%s select failed\n", FONT_RED, FONT_RESET);
      exit(1);
    }
    else if (ret != 0 && FD_ISSET(0, &fds)) {
      memset(buffer, '\0', BUFSIZE);
      fgets(buffer, BUFSIZE, stdin);
      chop(buffer);
      send(client->sock, buffer, BUFSIZE, 0);
      if (is_equal_str(buffer, "quit")) {
        printf("quit\n");
        state.is_active = false;
        break;
      }
    }
  }
  printf("handle_send exit\n");
  pthread_exit(NULL);
}

void *handle_receive(void *arg) {
  client_t *client = (client_t *) arg;
  char buffer[BUFSIZE];
  printf("handle_receive start\n");
  while (state.is_active) {
    memset(buffer, '\0', BUFSIZE);
    recv(client->sock, buffer, BUFSIZE, 0);
    printf("Received: %s\n", buffer);
    fflush(stdout);
    if (is_equal_str(buffer, "quit")) {
      printf("quit\n");
      state.is_active = false;
      break;
    }
  }
  printf("handle_receive exit\n");
  pthread_exit(NULL);
}