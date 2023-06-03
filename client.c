#include <stdio.h>
#include <stdlib.h>

#include "myutil.h"

// todo
// - ./client <host> <port>
// - サーバーに接続する
// - quit で退出する
// - メッセージを送る
// - メッセージを受け取る
// - 名前を入力できるようにする

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Usage: %s <host> <port>\n", argv[0]);
    return 1;
  }
  char* host = argv[1];
  int port = str2portNum(argv[2]);
  if (port == -1) {
    printf("Invalid port number: %s\n", argv[2]);
    return 1;
  }
  printf("Connecting to %s:%d\n", host, port);
  return 0;
}