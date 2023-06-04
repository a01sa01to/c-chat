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
#include "sock.h"

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
  bool send_terminated, recv_terminated;
} client_t;

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

  while (true) {
    // クライアントの接続を待つ
    client_t *client = &clients[num_clients];
    pthread_t *send_thread = &client->send_thread;
    pthread_t *receive_thread = &client->recv_thread;
    client->id = num_clients;
    num_clients++;

    memset((void *) client, 0, sizeof(*client));
    client->sock = accept(listening_socket, (struct sockaddr *) &client->addr, &(socklen_t) { sizeof(client->addr) });
    if (client->sock == -1) {
      printf("%serror%s accept failed\n", FONT_RED, FONT_RESET);
      exit(EXIT_FAILURE);
    }

    // クライアント接続された
    printf("%sinfo%s new connection from %s:%d\n", FONT_CYAN, FONT_RESET, inet_ntoa(client->addr.sin_addr), ntohs(client->addr.sin_port));
    printf("%sinfo%s number of clients: %d/%d\n", FONT_CYAN, FONT_RESET, num_clients, MAX_CLIENTS);

    // スレッドの作成
    if (pthread_create(send_thread, NULL, handle_send, (void *) &client->sock) != 0) {
      printf("%serror%s sender thread creation failed\n", FONT_RED, FONT_RESET);
      exit(EXIT_FAILURE);
    }
    if (pthread_create(receive_thread, NULL, handle_receive, (void *) &client->sock) != 0) {
      printf("%serror%s receiver thread creation failed\n", FONT_RED, FONT_RESET);
      exit(EXIT_FAILURE);
    }

    // もしクライアントが最大数に達したら終了
    if (num_clients == MAX_CLIENTS) {
      printf("%swarn%s max clients reached\n", FONT_YELLOW, FONT_RESET);
      break;
    }
  }

  // 終了処理
  printf("%sinfo%s closing server...\n", FONT_CYAN, FONT_RESET);
  close(listening_socket);
  for (int i = 0; i < num_clients; i++) pthread_join(clients[i].send_thread, NULL);
  printf("%ssuccess%s send threads joined\n", FONT_GREEN, FONT_RESET);
  for (int i = 0; i < num_clients; i++) pthread_join(clients[i].recv_thread, NULL);
  printf("%ssuccess%s receive threads joined\n", FONT_GREEN, FONT_RESET);
  for (int i = 0; i < num_clients; i++) close(clients[i].sock);
  printf("%ssuccess%s sockets closed\n", FONT_GREEN, FONT_RESET);
  exit(EXIT_SUCCESS);
}