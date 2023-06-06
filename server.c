// ---------- includes ---------- //
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "common/io.h"
#include "common/mleak.h"
#include "common/myutil.h"
#include "common/string.h"
#include "server_h/client.h"
#include "server_h/global_var.h"
#include "server_h/struct.h"

// ---------- main ---------- //
int main(int argc, char *argv[]) {
  atexit(mleak_finalize);

  // まずはコマンドが正しく入力されているかを確認する
  if (argc != 2) {
    printf("%serror%s invalid number of arguments\n", FONT_RED, FONT_RESET);
    printf("%sinfo%s usage: %s <port>\n", FONT_CYAN, FONT_RESET, argv[0]);
    exit(EXIT_FAILURE);
  }
  int port = str2portNum(argv[1]);
  if (port == -1) {
    printf("%serror%s invalid port number: %s\n", FONT_RED, FONT_RESET, argv[1]);
    exit(EXIT_FAILURE);
  }
  printf("%sinfo%s listening on port %d\n", FONT_CYAN, FONT_RESET, port);

  // グローバル変数の初期
  message.content = (string *) malloc(sizeof(string));
  message.sender_name = (string *) malloc(sizeof(string));
  string__init(message.content);
  string__init(message.sender_name);
  message.sender_id = -1;
  message.message_id = -1;

  // listening socketの作成
  int listening_socket = socket(PF_INET, SOCK_STREAM, 0);
  if (listening_socket == -1) {
    printf("%serror%s socket creation failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }
  // ソケットのオプション設定
  if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 }, sizeof(int))) {
    printf("%serror%s socket option setting failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  // サーバー設定
  struct sockaddr_in server;
  memset((void *) &server, 0, sizeof(server));
  server.sin_family = PF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind
  if (bind(listening_socket, (struct sockaddr *) &server, sizeof(server)) == -1) {
    printf("%serror%s bind failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }
  // Listen
  if (listen(listening_socket, MAX_CLIENTS) == -1) {
    printf("%serror%s listen failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  client_t clients[MAX_CLIENTS];
  int num_clients = 0;
  // 初期化
  for (int i = 0; i < MAX_CLIENTS; i++) {
    clients[i].addr = (struct sockaddr_in) { 0 };
    clients[i].sock = -1;
    clients[i].name = (string *) malloc(sizeof(string));
    string__from_cstr(clients[i].name, "anonymous");
    clients[i].send_thread = (pthread_t) { 0 };
    clients[i].recv_thread = (pthread_t) { 0 };
    clients[i].id = -1;
    clients[i].last_message_id = -1;
    clients[i].send_created = false;
    clients[i].recv_created = false;
    clients[i].send_terminated = false;
    clients[i].recv_terminated = false;
  }

  // クライアント接続スレッド
  pthread_t client_handler;
  client_handler_arg client_handler_arg = { &num_clients, clients, &listening_socket };
  if (pthread_create(&client_handler, NULL, handle_client, &client_handler_arg)) {
    printf("%serror%s failed to create client handler thread\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  // どれかが終了したか監視
  while (true) {
    bool any_terminated = false;
    for (int i = 0; i < num_clients; i++) {
      if (clients[i].send_terminated || clients[i].recv_terminated) {
        any_terminated = true;
        break;
      }
    }
    if (any_terminated) break;
  }

  // 終了処理
  printf("%sinfo%s closing server...\n", FONT_CYAN, FONT_RESET);
  if (pthread_cancel(client_handler) != 0) printf("%swarn%s failed to cancel client handler thread\n", FONT_YELLOW, FONT_RESET);
  if (pthread_join(client_handler, NULL) != 0) printf("%swarn%s failed to join client handler thread\n", FONT_YELLOW, FONT_RESET);
  for (int i = 0; i < num_clients; i++) {
    if ((clients[i].send_created && !clients[i].send_terminated) && pthread_join(clients[i].send_thread, NULL) != 0) printf("%swarn%s failed to join send thread for client %d\n", FONT_YELLOW, FONT_RESET, clients[i].id);
    if ((clients[i].recv_created && !clients[i].recv_terminated) && pthread_join(clients[i].recv_thread, NULL) != 0) printf("%swarn%s failed to join recv thread for client %d\n", FONT_YELLOW, FONT_RESET, clients[i].id);
  }
  close(listening_socket);
  for (int i = 0; i < MAX_CLIENTS; i++) {
    string__free(clients[i].name);
    if (clients[i].sock != -1) close(clients[i].sock);
  }
  string__free(message.content);
  string__free(message.sender_name);

  exit(EXIT_SUCCESS);
}
