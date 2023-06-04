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

// todo
// - ./client <host> <port>
// - サーバーに接続する
// - quit で退出する
// - メッセージを送る
// - メッセージを受け取る
// - 名前を入力できるようにする

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("%sinfo%s usage: %s <host> <port>\n", COLOR_CYAN, COLOR_RESET, argv[0]);
    return 1;
  }
  char *host = argv[1];
  int port = str2portNum(argv[2]);
  if (port == -1) {
    printf("%serror%s invalid port number: %s\n", COLOR_RED, COLOR_RESET, argv[2]);
    return 1;
  }
  printf("%sinfo%s connecting to %s:%d\n", COLOR_CYAN, COLOR_RESET, host, port);

  struct hostent *hostent = gethostbyname(host);
  if (hostent == NULL) {
    printf("%serror%s gethostbyname failed", COLOR_RED, COLOR_RESET);
    return 1;
  }

  struct sockaddr_in server;
  memset((void *) &server, 0, sizeof(server));
  server.sin_family = PF_INET;
  server.sin_port = htons(port);
  memcpy((void *) &server.sin_addr, (void *) hostent->h_addr, hostent->h_length);

  client_t client;
  client.sock = socket(PF_INET, SOCK_STREAM, 0);
  if (client.sock == -1) {
    printf("%serror%s socket creation failed", COLOR_RED, COLOR_RESET);
    return 1;
  }
  if (connect(client.sock, (struct sockaddr *) &server, sizeof(server)) == -1) {
    printf("%serror%s connect failed", COLOR_RED, COLOR_RESET);
    return 1;
  }

  printf("%sinfo%s connected\n", COLOR_CYAN, COLOR_RESET);

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