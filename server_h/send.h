#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "../common/io.h"
#include "global_var.h"
#include "struct.h"

// 送信用
void *handle_send(void *arg) {
  client_t *client = (client_t *) arg;
  char buffer[BUFSIZE];
  client->send_created = true;
  while (true) {
    // 送信するべきデータがない場合は待機
    if (message.message_id == client->last_message_id) continue;

    // 送信データの構築
    memset(buffer, '\0', BUFSIZE);
    if (message.sender_id == client->id) {
      sprintf(buffer, "%s%s%s%s\n%s", message.sender_name, FONT_BOLD, " (me)", FONT_RESET, message.content);
    }
    else {
      sprintf(buffer, "%s\n%s", message.sender_name, message.content);
    }

    // 送信する
    send(client->sock, buffer, BUFSIZE, 0);

    if (is_equal_str(message.content, "quit")) {
      printf("%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      break;
    }

    // 送信したメッセージのIDを更新
    client->last_message_id = message.message_id;
  }
  client->send_terminated = true;
  pthread_exit(NULL);
}
