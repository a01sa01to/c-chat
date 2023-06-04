#pragma once

#include <arpa/inet.h>
#include <stdio.h>

#include "receive.h"
#include "send.h"
#include "struct.h"

#define MAX_CLIENTS 10

typedef struct {
  int *num_clients;
  client_t *clients;
  int *listening_socket;
} client_handler_arg;

void *handle_client(void *arg) {
  client_t *clients = ((client_handler_arg *) arg)->clients;
  int *num_clients = ((client_handler_arg *) arg)->num_clients;
  int *listening_socket = ((client_handler_arg *) arg)->listening_socket;

  while (true) {
    // クライアントの接続を待つ
    client_t *client = &clients[*num_clients];
    pthread_t *send_thread = &client->send_thread;
    pthread_t *receive_thread = &client->recv_thread;
    client->id = *num_clients;
    (*num_clients)++;

    client->sock = accept(*listening_socket, (struct sockaddr *) &client->addr, &(socklen_t) { sizeof(client->addr) });
    if (client->sock == -1) {
      printf("%serror%s accept failed\n", FONT_RED, FONT_RESET);
      exit(EXIT_FAILURE);
    }

    // クライアント接続された
    printf("%sinfo%s new connection from %s:%d\n", FONT_CYAN, FONT_RESET, inet_ntoa(client->addr.sin_addr), ntohs(client->addr.sin_port));
    printf("%sinfo%s number of clients: %d/%d\n", FONT_CYAN, FONT_RESET, *num_clients, MAX_CLIENTS);

    // スレッドの作成
    if (pthread_create(send_thread, NULL, handle_send, (void *) client) != 0) {
      printf("%serror%s sender thread creation failed\n", FONT_RED, FONT_RESET);
      exit(EXIT_FAILURE);
    }
    if (pthread_create(receive_thread, NULL, handle_receive, (void *) client) != 0) {
      printf("%serror%s receiver thread creation failed\n", FONT_RED, FONT_RESET);
      exit(EXIT_FAILURE);
    }
    client->send_created = true;
    client->recv_created = true;

    // もしクライアントが最大数に達したら終了
    if (*num_clients == MAX_CLIENTS) {
      printf("%swarn%s max clients reached\n", FONT_YELLOW, FONT_RESET);
      break;
    }
  }
  pthread_exit(NULL);
}
