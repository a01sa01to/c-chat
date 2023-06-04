#pragma once

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>

#include "../common/io.h"

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
    send(*sock, buffer, BUFSIZE, 0);

    // 一行戻し、行を削除
    printf("\033[1A\033[2K");

    // 終了判定は受信側で行う
  }
  pthread_exit(NULL);
}
