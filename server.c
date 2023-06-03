#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "myutil.h"

// todo
// - ./server <port>
// - ポートで待ち受ける
// - スレッドを作る
// - quit で終了する
// - 複数クライアントに対応する
// - ユーザー名の管理

int main(int argc, char* argv[]) {
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
  memset((void*) &server, 0, sizeof(server));
  server.sin_family = PF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind
  return 0;
}