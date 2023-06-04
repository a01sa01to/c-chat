#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "io.h"
#include "myutil.h"
#include "sock.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("%sinfo%s usage: %s <host> <port>\n", FONT_CYAN, FONT_RESET, argv[0]);
    exit(EXIT_FAILURE);
  }
  char *host = argv[1];
  int port = str2portNum(argv[2]);
  if (port == -1) {
    printf("%serror%s invalid port number: %s\n", FONT_RED, FONT_RESET, argv[2]);
    exit(EXIT_FAILURE);
  }
  printf("%sinfo%s connecting to %s:%d\n", FONT_CYAN, FONT_RESET, host, port);

  struct hostent *hostent = gethostbyname(host);
  if (hostent == NULL) {
    printf("%serror%s gethostbyname failed", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server;
  memset((void *) &server, 0, sizeof(server));
  server.sin_family = PF_INET;
  server.sin_port = htons(port);
  memcpy((void *) &server.sin_addr, (void *) hostent->h_addr, hostent->h_length);

  client_t client;
  client.sock = socket(PF_INET, SOCK_STREAM, 0);
  if (client.sock == -1) {
    printf("%serror%s socket creation failed", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }
  if (connect(client.sock, (struct sockaddr *) &server, sizeof(server)) == -1) {
    printf("%serror%s connect failed", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  printf("%sinfo%s connected\n", FONT_CYAN, FONT_RESET);

  printf("%stips%s send %s/help%s to show how to use.\n", FONT_PURPLE, FONT_RESET, FONT_BOLD, FONT_RESET);
  printf("\n\r> ");
  fflush(stdout);

  // create thread
  pthread_t send_thread, receive_thread;
  if (pthread_create(&send_thread, NULL, handle_send, (void *) &client) != 0) {
    printf("%serror%s sender pthread_create failed", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }
  if (pthread_create(&receive_thread, NULL, handle_receive, (void *) &client) != 0) {
    printf("%serror%s receiver pthread_create failed", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  // join thread
  if (pthread_join(send_thread, NULL) != 0) {
    printf("%serror%s sender pthread_join failed", FONT_RED, FONT_RESET);
  }
  if (pthread_join(receive_thread, NULL) != 0) {
    printf("%serror%s receiver pthread_join failed", FONT_RED, FONT_RESET);
  }

  close(client.sock);
  exit(EXIT_SUCCESS);
}