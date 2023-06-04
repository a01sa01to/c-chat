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

// todo
// - ./server <port>
// - ポートで待ち受ける
// - スレッドを作る
// - quit で終了する
// - 複数クライアントに対応する
// - ユーザー名の管理

int main(int argc, char *argv[]) {
  // Input validation
  if (argc != 2) {
    printf("%sinfo%s usage: %s <port>\n", COLOR_CYAN, COLOR_RESET, argv[0]);
    return 1;
  }
  int port = str2portNum(argv[1]);
  if (port == -1) {
    printf("%serror%s invalid port number: %s\n", COLOR_RED, COLOR_RESET, argv[1]);
    return 1;
  }
  printf("%sinfo%s listening on port %d\n", COLOR_CYAN, COLOR_RESET, port);

  // Create a socket
  int listening_socket = socket(PF_INET, SOCK_STREAM, 0);
  if (listening_socket == -1) {
    printf("%serror%s socket creation failed", COLOR_RED, COLOR_RESET);
    return 1;
  }
  // Socket option
  if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 }, sizeof(int))) {
    printf("%serror%s socket option failed", COLOR_RED, COLOR_RESET);
    return 1;
  }

  // Server Socket Address
  struct sockaddr_in server;
  memset((void *) &server, 0, sizeof(server));
  server.sin_family = PF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind
  if (bind(listening_socket, (struct sockaddr *) &server, sizeof(server)) == -1) {
    printf("%serror%s bind failed", COLOR_RED, COLOR_RESET);
    return 1;
  }

  // Listen
  if (listen(listening_socket, 5) == -1) {
    printf("%serror%s listen failed", COLOR_RED, COLOR_RESET);
    return 1;
  }
  client_t client;
  memset((void *) &client, 0, sizeof(client));
  client.sock = accept(listening_socket, (struct sockaddr *) &client.addr, &(socklen_t) { sizeof(client.addr) });
  if (client.sock == -1) {
    printf("%serror%s accept failed", COLOR_RED, COLOR_RESET);
    return 1;
  }

  printf("%sinfo%s connected from %s:%d\n", COLOR_CYAN, COLOR_RESET, inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));
  close(listening_socket);

  // create thread
  pthread_t send_thread, receive_thread;
  if (pthread_create(&send_thread, NULL, handle_send, (void *) &client) != 0) {
    printf("%serror%s sender pthread_create failed", COLOR_RED, COLOR_RESET);
    return 1;
  }
  if (pthread_create(&receive_thread, NULL, handle_receive, (void *) &client) != 0) {
    printf("%serror%s receiver pthread_create failed", COLOR_RED, COLOR_RESET);
    return 1;
  }

  // join thread
  if (pthread_join(send_thread, NULL) != 0) {
    printf("%serror%s sender pthread_join failed", COLOR_RED, COLOR_RESET);
  }
  if (pthread_join(receive_thread, NULL) != 0) {
    printf("%serror%s receiver pthread_join failed", COLOR_RED, COLOR_RESET);
  }

  close(client.sock);
  return 0;
}
