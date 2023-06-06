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
  client->send_created = true;
  while (true) {
    // 送信するべきデータがない場合は待機
    if (message.message_id == client->last_message_id) continue;

    pthread_mutex_lock(message.mutex);

    // 送信データの構築
    string *res;
    string *username;
    string *me;
    string *br;
    string__init(&res);
    string__copy(&username, message.sender_name);
    string__from_cstr(&me, " (me)");
    string__from_cstr(&br, "\n");

    string__append(res, username);
    if (message.sender_id == client->id) string__append(res, me);
    string__append(res, br);
    string__append(res, message.content);

    // string -> char*
    char *buffer;
    string2cstr(&buffer, res);

    // 送信する
    send(client->sock, buffer, res->length + 1, 0);

    // free
    string__free(&res);
    string__free(&username);
    string__free(&me);
    string__free(&br);
    free(buffer);

    // 終了判定
    string *quit;
    string__from_cstr(&quit, "quit");

    if (is_equal_str(message.content, quit)) {
      printf("%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      string__free(&quit);
      pthread_mutex_unlock(message.mutex);
      break;
    }
    string__free(&quit);

    // 送信したメッセージのIDを更新
    client->last_message_id = message.message_id;

    pthread_mutex_unlock(message.mutex);
  }
  client->send_terminated = true;
  pthread_exit(NULL);
}
