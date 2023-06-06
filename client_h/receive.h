#pragma once

#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "../common/io.h"
#include "../common/myutil.h"
#include "../common/string.h"

// 受信用
void* handle_receive(void* arg) {
  int* sock = (int*) arg;
  char buffer[BUFSIZE];
  while (true) {
    // 受信
    memset(buffer, '\0', BUFSIZE);
    recv(*sock, buffer, BUFSIZE, 0);

    string* buf;
    string__from_cstr(&buf, buffer);

    string* username;
    string* message;
    decode_username(&username, buf);
    decode_message(&message, buf);

    char* username_cstr;
    char* message_cstr;
    string2cstr(&username_cstr, username);
    string2cstr(&message_cstr, message);

    // ユーザー名を下線付きで表示
    printf("\r%s%s%s\n", FONT_UNDERLINED, username_cstr, FONT_RESET);
    // メッセージを表示
    printf(">> %s\n", message_cstr);

    // free
    string__free(&buf);
    string__free(&username);
    string__free(&message);
    free(username_cstr);

    // 終了判定
    if (strcmp(message_cstr, "quit") == 0) {
      free(message_cstr);
      printf("\n%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      break;
    }
    free(message_cstr);

    // プロンプトを表示
    printf("\n\r> ");
    fflush(stdout);
  }
  pthread_exit(NULL);
}