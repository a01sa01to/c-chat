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
  while (state.is_active) {
    memset(buffer, '\0', BUFSIZE);
    fgets(buffer, BUFSIZE, stdin);
    send(client->sock, buffer, BUFSIZE, 0);
    if (is_equal_str(buffer, "quit")) {
      printf("quit\n");
      state.is_active = false;
      break;
    }
  }
  printf("handle_send exit\n");
  pthread_exit(NULL);
}

void *handle_receive(void *arg) {
  client_t *client = (client_t *) arg;
  char buffer[BUFSIZE];
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