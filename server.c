#include <stdio.h>
#include <stdlib.h>

// todo
// - ./server <port>
// - ポートで待ち受ける
// - スレッドを作る
// - quit で終了する
// - 複数クライアントに対応する
// - ユーザー名の管理

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  printf("Listening on port %d\n", port);
  return 0;
}