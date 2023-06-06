#pragma once

#include <arpa/inet.h>
#include <stdio.h>

#include "../common/io.h"
#include "../common/myutil.h"
#include "global_var.h"
#include "struct.h"

void send_system_message(client_t *client, char *msg) {
  char message[BUFSIZE];
  sprintf(message, "%s\n%s", "System (only shown to you)", msg);
  send(client->sock, message, BUFSIZE, 0);
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

    // ユーザー名を下線付きで表示
    printf("\r%s%s%s [%s:%d]\n", FONT_UNDERLINED, client->name, FONT_RESET, inet_ntoa(client->addr.sin_addr), ntohs(client->addr.sin_port));
    // メッセージを表示
    printf(">> %s\n\n", buffer);
    fflush(stdout);

    // コマンドの処理
    if (buffer[0] == '/') {
      if (str_startsWith(buffer, "/setname ")) {
        // ユーザー名の変更
        char *new_name = buffer + strlen("/setname ");
        if (strlen(new_name) > 0 && strlen(new_name) < NAME_LEN) {
          strcpy(client->name, new_name);
          send_system_message(client, "Your name has been changed");
        }
        else {
          send_system_message(client, "Invalid name");
        }
      }
      else if (str_startsWith(buffer, "/list")) {
        // todo
        send_system_message(client, "Not implemented yet");
      }
      else if (str_startsWith(buffer, "/help")) {
        send_system_message(client, "Available commands:\n>> - /setname <name>: Change your name\n>> - /list: Show people in this chat\n>> - /help: Show this message");
      }
      else {
        send_system_message(client, "Command not found");
      }
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
      should_exit = true;
      break;
    }
  }
  client->recv_terminated = true;
  pthread_exit(NULL);
}