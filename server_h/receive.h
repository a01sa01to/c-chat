#pragma once

#include <stdio.h>

#include "../common/io.h"
#include "../common/myutil.h"
#include "global_msg.h"
#include "struct.h"

// 受信用
void *handle_receive(void *arg) {
  client_t *client = (client_t *) arg;
  char buffer[BUFSIZE];
  while (true) {
    // 受信
    memset(buffer, '\0', BUFSIZE);
    recv(client->sock, buffer, BUFSIZE, 0);

    // ユーザー名を下線付きで表示
    printf("\r%s%s%s\n", FONT_UNDERLINED, client->name, FONT_RESET);
    // メッセージを表示
    printf(">> %s\n\n", buffer);
    fflush(stdout);

    // コマンドの処理
    if (buffer[0] == '/') {
    }
    // 通常のメッセージ
    else {
      strcpy(message.content, buffer);
      message.sender_id = client->id;
      strcpy(message.sender_name, client->name);
      message.message_id++;
    }

    // 終了判定
    if (is_equal_str(buffer, "quit")) {
      printf("%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      break;
    }
  }
  pthread_exit(NULL);
}