// ---------- includes ---------- //
#include <stdio.h>
#include <unistd.h>

#include "common/io.h"
#include "common/myutil.h"
#include "server_h/client.h"
#include "server_h/global_msg.h"
#include "server_h/struct.h"

// ---------- main ---------- //
int main(int argc, char *argv[]) {
  // まずはコマンドが正しく入力されているかを確認する
  if (argc != 2) {
    printf("%sinfo%s usage: %s <port>\n", FONT_CYAN, FONT_RESET, argv[0]);
    exit(EXIT_FAILURE);
  }
  int port = str2portNum(argv[1]);
  if (port == -1) {
    printf("%serror%s invalid port number: %s\n", FONT_RED, FONT_RESET, argv[1]);
    exit(EXIT_FAILURE);
  }
  printf("%sinfo%s listening on port %d\n", FONT_CYAN, FONT_RESET, port);

  // グローバル変数の初期化
  memset(message.content, '\0', BUFSIZE);
  memset(message.sender_name, '\0', NAME_LEN);
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
    strcpy(clients[i].name, "anonymous");
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

  // 各スレッドの監視
  bool client_handler_terminated = false;
  while (true) {
    // 終了しているか確認
    bool all_terminated = client_handler_terminated;
    for (int i = 0; i < num_clients; i++) {
      if ((clients[i].send_created && !clients[i].send_terminated) || (clients[i].recv_created && !clients[i].recv_terminated)) {
        all_terminated = false;
        break;
      }
    }
    if (all_terminated) {
      printf("%ssuccess%s all threads terminated\n", FONT_GREEN, FONT_RESET);
      break;
    }

    // どれかが終了しているかチェック
    bool any_terminated = client_handler_terminated;
    for (int i = 0; i < num_clients; i++) {
      if (clients[i].send_terminated || clients[i].recv_terminated) {
        any_terminated = true;
        break;
      }
    }

    // 最新の状態に更新する
    if (!client_handler_terminated) {
      if (pthread_tryjoin_np(client_handler, NULL) == 0) {
        client_handler_terminated = true;
        printf("%ssuccess%s client handler thread terminated\n", FONT_GREEN, FONT_RESET);
      }
    }
    for (int i = 0; i < num_clients; i++) {
      if (clients[i].send_created && !clients[i].send_terminated) {
        if (pthread_tryjoin_np(clients[i].send_thread, NULL) == 0) {
          clients[i].send_terminated = true;
          printf("%ssuccess%s send thread for client %d terminated\n", FONT_GREEN, FONT_RESET, clients[i].id);
        }
      }
      if (clients[i].recv_created && !clients[i].recv_terminated) {
        if (pthread_tryjoin_np(clients[i].recv_thread, NULL) == 0) {
          clients[i].recv_terminated = true;
          printf("%ssuccess%s recv thread for client %d terminated\n", FONT_GREEN, FONT_RESET, clients[i].id);
        }
      }
    }

    // もしどれかが終了していたら終了処理
    if (any_terminated) {
      if (!client_handler_terminated) {
        if (pthread_cancel(client_handler) != 0) {
          printf("%serror%s failed to cancel client handler thread\n", FONT_RED, FONT_RESET);
        }
      }

      for (int i = 0; i < num_clients; i++) {
        if (clients[i].send_created && !clients[i].send_terminated) {
          if (pthread_cancel(clients[i].send_thread) != 0) {
            printf("%serror%s failed to cancel send thread for client %d\n", FONT_RED, FONT_RESET, clients[i].id);
          }
        }
        if (clients[i].recv_created && !clients[i].recv_terminated) {
          if (pthread_cancel(clients[i].recv_thread) != 0) {
            printf("%serror%s failed to cancel recv thread for client %d\n", FONT_RED, FONT_RESET, clients[i].id);
          }
        }
      }
    }
  }

  // 終了処理
  printf("%sinfo%s closing server...\n", FONT_CYAN, FONT_RESET);
  close(listening_socket);
  for (int i = 0; i < num_clients; i++) close(clients[i].sock);
  printf("%ssuccess%s sockets closed\n", FONT_GREEN, FONT_RESET);
  exit(EXIT_SUCCESS);
}
