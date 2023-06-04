#pragma once

#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "../common/io.h"
#include "../common/myutil.h"

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

    // 終了判定
    if (is_equal_str(decode_message(buffer), "quit")) {
      printf("\n%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      break;
    }

    // プロンプトを表示
    printf("\n\r> ");
    fflush(stdout);
  }
  pthread_exit(NULL);
}