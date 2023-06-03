#include <stdio.h>
#include <stdlib.h>

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
  int port = atoi(argv[2]);
  printf("Connecting to %s:%d\n", host, port);
  return 0;
}