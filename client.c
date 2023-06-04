#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "myutil.h"

void *handle_send(void *arg);
void *handle_receive(void *arg);

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

  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    printf("%serror%s socket creation failed", COLOR_RED, COLOR_RESET);
    return 1;
  }

  struct sockaddr_in server;
  memset((void *) &server, 0, sizeof(server));
  server.sin_family = PF_INET;
  server.sin_port = htons(port);
  struct hostent *hostent = gethostbyname(host);
  if (hostent == NULL) {
    printf("%serror%s gethostbyname failed", COLOR_RED, COLOR_RESET);
    return 1;
  }
  memcpy((void *) &server.sin_addr, (void *) hostent->h_addr, hostent->h_length);

  // connect
  if (connect(sock, (struct sockaddr *) &server, sizeof(server)) == -1) {
    printf("%serror%s connect failed", COLOR_RED, COLOR_RESET);
    return 1;
  }

  printf("%sinfo%s connected\n", COLOR_CYAN, COLOR_RESET);

  // todo
  send(sock, "quit", 4, 0);

  return 0;
}

void *handle_send(void *arg) {
  // todo
  printf("handle_send\n");
  pthread_exit(NULL);
}

void *handle_receive(void *arg) {
  // todo
  printf("handle_receive\n");
  pthread_exit(NULL);
}