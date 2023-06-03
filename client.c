#include <stdio.h>
#include <stdlib.h>

#include "io.h"
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
    printf("%sinfo%s usage: %s <host> <port>\n", COLOR_CYAN, COLOR_RESET, argv[0]);
    return 1;
  }
  char* host = argv[1];
  int port = str2portNum(argv[2]);
  if (port == -1) {
    printf("%serror%s invalid port number: %s\n", COLOR_RED, COLOR_RESET, argv[2]);
    return 1;
  }
  printf("%sinfo%s connecting to %s:%d\n", COLOR_CYAN, COLOR_RESET, host, port);

  return 0;
}