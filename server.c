// ---------- includes ---------- //
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "io.h"
#include "myutil.h"

// ---------- global variables ---------- //
#define MAX_CLIENTS 10
#define NAME_LEN 20

// ---------- structs ---------- //
typedef struct {
  struct sockaddr_in addr;
  int sock;
  char name[NAME_LEN];
  pthread_t send_thread, recv_thread;
  int id;
  int last_message_id;
  bool send_created, recv_created;
  bool send_terminated, recv_terminated;
} client_t;

typedef struct {
  int *num_clients;
  client_t *clients;
  int *listening_socket;
} client_handler_arg;

typedef struct {
  char *buf;
  char *sender_name;
  int sender_id;
  int message_id;
} message_state;

// ---------- global variables ---------- //
message_state message = { NULL, NULL, -1, -1 };

// ---------- handler function prototypes ---------- //
void *handle_client(void *arg);
void *handle_send(void *arg);
void *handle_receive(void *arg);

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

// ---------- handler function implementations ---------- //
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

// 送信用
void *handle_send(void *arg) {
  client_t *client = (client_t *) arg;
  char buffer[BUFSIZE];

  while (true) {
    // 入力を受け取る
    memset(buffer, '\0', BUFSIZE);
    fgets(buffer, BUFSIZE, stdin);
    chop(buffer);

    // 送信する
    send(client->sock, encode("anonymous", buffer), BUFSIZE, 0);

    // 終了判定 (to be removed)
    if (is_equal_str(buffer, "quit")) break;

    // プロンプトを表示 (to be removed)
    printf("> ");
  }
  pthread_exit(NULL);
}

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
    printf(">> %s\n", buffer);
    // プロンプトを表示
    printf("\n\r> ");
    fflush(stdout);

    // 終了判定
    if (is_equal_str(buffer, "quit")) {
      printf("%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      break;
    }

    // コマンドの処理
    if (buffer[0] == '/') {
    }
    // 通常のメッセージ
    else {
      message.buf = buffer;
      message.sender_id = client->id;
      message.sender_name = client->name;
      message.message_id++;
    }
  }
  pthread_exit(NULL);
}