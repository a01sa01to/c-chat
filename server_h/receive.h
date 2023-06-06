#pragma once

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "../common/io.h"
#include "../common/myutil.h"
#include "../common/string.h"
#include "global_var.h"
#include "struct.h"

void send_system_message(client_t *client, string *msg) {
  // メッセージを作成
  string *res;
  string *username;
  string *br;
  string__init(&res);
  string__from_cstr(&username, "System (only shown to you)");
  string__from_cstr(&br, "\n");
  string__append(res, username);
  string__append(res, br);
  string__append(res, msg);

  // string -> char*
  char *message;
  string2cstr(&message, res);

  // 送信
  send(client->sock, message, res->length + 1, 0);

  // free
  string__free(&res);
  string__free(&username);
  string__free(&br);
  free(message);
}

// 受信用
void *handle_receive(void *arg) {
  client_t *client = (client_t *) arg;
  char buffer[BUFSIZE];
  client->recv_created = true;
  while (!should_exit) {
    // 受信
    memset(buffer, '\0', BUFSIZE);
    recv(client->sock, buffer, BUFSIZE, 0);

    char *name;
    string2cstr(&name, client->name);

    // ユーザー名を下線付きで表示
    printf("\r%s%s%s [%s:%d]\n", FONT_UNDERLINED, name, FONT_RESET, inet_ntoa(client->addr.sin_addr), ntohs(client->addr.sin_port));
    free(name);

    // メッセージを表示
    printf(">> %s\n\n", buffer);
    fflush(stdout);

    // コマンドの処理
    if (buffer[0] == '/') {
      // 返すstring
      string *msg;
      if (str_startsWith(buffer, "/setname ")) {
        // ユーザー名の変更
        char *new_name = buffer + strlen("/setname ");
        if (strlen(new_name) > 0) {
          string__free(&client->name);
          string__from_cstr(&client->name, new_name);
          string__from_cstr(&msg, "Your name has been changed");
        }
        else {
          string__from_cstr(&msg, "Invalid name");
        }
      }
      else if (str_startsWith(buffer, "/help")) {
        string__from_cstr(&msg, "Available commands:\n>> - /setname <name>: Change your name\n>> - /help: Show this message");
      }
      else {
        string__from_cstr(&msg, "Command not found");
      }
      send_system_message(client, msg);
      string__free(&msg);
    }
    // 通常のメッセージ
    else {
      pthread_mutex_lock(message.mutex);

      // Messageに書き込み
      string__free(&message.content);
      string__free(&message.sender_name);
      string__from_cstr(&message.content, buffer);
      string__copy(&message.sender_name, client->name);
      message.sender_id = client->id;
      message.message_id++;

      pthread_mutex_unlock(message.mutex);
    }

    // 終了判定
    if (strcmp(buffer, "quit") == 0) {
      printf("%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      should_exit = true;
      break;
    }
  }
  client->recv_terminated = true;
  pthread_exit(NULL);
}