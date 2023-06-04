#include <arpa/inet.h>
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
  // Input validation
  if (argc != 2) {
    printf("%sinfo%s usage: %s <port>\n", FONT_CYAN, FONT_RESET, argv[0]);
    exit(EXIT_FAILURE);
  }
  int port = str2portNum(argv[1]);
  if (port == -1) {
    printf("%serror%s invalid port number: %s\n", FONT_RED, FONT_RESET, argv[1]);
    exit(EXIT_FAILURE);
  }
  printf("%sinfo%s listening on port %d\n", FONT_CYAN, FONT_RESET, port);

  // Create a socket
  int listening_socket = socket(PF_INET, SOCK_STREAM, 0);
  if (listening_socket == -1) {
    printf("%serror%s socket creation failed", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }
  // Socket option
  if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 }, sizeof(int))) {
    printf("%serror%s socket option failed", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  // Server Socket Address
  struct sockaddr_in server;
  memset((void *) &server, 0, sizeof(server));
  server.sin_family = PF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind
  if (bind(listening_socket, (struct sockaddr *) &server, sizeof(server)) == -1) {
    printf("%serror%s bind failed", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  // Listen
  if (listen(listening_socket, 5) == -1) {
    printf("%serror%s listen failed", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }
  client_t client;
  memset((void *) &client, 0, sizeof(client));
  client.sock = accept(listening_socket, (struct sockaddr *) &client.addr, &(socklen_t) { sizeof(client.addr) });
  if (client.sock == -1) {
    printf("%serror%s accept failed", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  printf("%sinfo%s connected from %s:%d\n", FONT_CYAN, FONT_RESET, inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));
  close(listening_socket);

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
