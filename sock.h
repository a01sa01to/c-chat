#pragma once

#include <netinet/in.h>
#include <pthread.h>

#include "io.h"

// 送信用
void *handle_send(void *arg) {
  int *sock = (int *) arg;
  char buffer[BUFSIZE];

  while (true) {
    // 入力を受け取る
    memset(buffer, '\0', BUFSIZE);
    fgets(buffer, BUFSIZE, stdin);
    chop(buffer);

    // 送信する
    send(*sock, encode("anonymous", buffer), BUFSIZE, 0);

    // 終了判定 (to be removed)
    if (is_equal_str(buffer, "quit")) break;

    // プロンプトを表示 (to be removed)
    printf("> ");
  }
  pthread_exit(NULL);
}

// 受信用
void *handle_receive(void *arg) {
  int *sock = (int *) arg;
  char buffer[BUFSIZE];
  while (true) {
    // 受信
    memset(buffer, '\0', BUFSIZE);
    recv(*sock, buffer, BUFSIZE, 0);

    // ユーザー名を下線付きで表示
    printf("\r%s%s%s\n", FONT_UNDERLINED, decode_username(buffer), FONT_RESET);
    // メッセージを表示
    printf(">> %s\n", decode_message(buffer));
    // プロンプトを表示
    printf("\n\r> ");
    fflush(stdout);

    // 終了判定
    if (is_equal_str(decode_message(buffer), "quit")) {
      printf("%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      break;
    }
  }
  pthread_exit(NULL);
}