#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "io.h"
#include "myutil.h"

// ---------- handler function prototypes ---------- //
void *handle_send(void *arg);
void *handle_receive(void *arg);

// ---------- main function ---------- //
int main(int argc, char *argv[]) {
  // まずはコマンドが正しく入力されているかを確認する
  if (argc != 3) {
    printf("%sinfo%s usage: %s <host> <port>\n", FONT_CYAN, FONT_RESET, argv[0]);
    exit(EXIT_FAILURE);
  }
  char *host = argv[1];
  int port = str2portNum(argv[2]);
  if (port == -1) {
    printf("%serror%s invalid port number: %s\n", FONT_RED, FONT_RESET, argv[2]);
    exit(EXIT_FAILURE);
  }
  printf("%sinfo%s connecting to %s:%d\n", FONT_CYAN, FONT_RESET, host, port);
  struct hostent *hostent = gethostbyname(host);
  if (hostent == NULL) {
    printf("%serror%s gethostbyname failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  // server の情報
  struct sockaddr_in server;
  memset((void *) &server, 0, sizeof(server));
  server.sin_family = PF_INET;
  server.sin_port = htons(port);
  memcpy((void *) &server.sin_addr, (void *) hostent->h_addr, hostent->h_length);

  // socket の作成
  int sock;
  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    printf("%serror%s socket creation failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  // 接続
  if (connect(sock, (struct sockaddr *) &server, sizeof(server)) == -1) {
    printf("%serror%s connection failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }
  printf("%sinfo%s connected\n", FONT_CYAN, FONT_RESET);
  printf("%stips%s send %s/help%s to show how to use\n", FONT_PURPLE, FONT_RESET, FONT_BOLD, FONT_RESET);
  printf("\n\r> ");
  fflush(stdout);

  // スレッドを作る
  pthread_t send_thread, receive_thread;
  if (pthread_create(&send_thread, NULL, handle_send, (void *) &sock) != 0) {
    printf("%serror%s sender thread creation failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }
  if (pthread_create(&receive_thread, NULL, handle_receive, (void *) &sock) != 0) {
    printf("%serror%s receiver thread creation failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  // スレッドの監視
  bool sender_terminated = false, receiver_terminated = false;
  while (true) {
    // もし両方のスレッドが終了したら終了する
    if (sender_terminated && receiver_terminated) break;

    // 送信側
    if (pthread_tryjoin_np(send_thread, NULL) == 0) {
      sender_terminated = true;
      printf("%ssuccess%s sender thread was terminated\n", FONT_GREEN, FONT_RESET);

      // 受信側を終了させる
      if (!receiver_terminated) {
        printf("%sinfo%s trying to terminate receive thread\n", FONT_CYAN, FONT_RESET);
        if (pthread_cancel(receive_thread) != 0) printf("%serror%s receiver thread cancellation failed\n", FONT_RED, FONT_RESET);
      }
    }

    // 受信側
    if (pthread_tryjoin_np(receive_thread, NULL) == 0) {
      receiver_terminated = true;
      printf("%ssuccess%s receive thread was terminated\n", FONT_GREEN, FONT_RESET);

      // 送信側を終了させる
      if (!sender_terminated) {
        printf("%sinfo%s trying to terminate sender thread\n", FONT_CYAN, FONT_RESET);
        if (pthread_cancel(send_thread) != 0) printf("%serror%s sender thread cancellation failed\n", FONT_RED, FONT_RESET);
      }
    }
  }
  close(sock);
  exit(EXIT_SUCCESS);
}

// ---------- handler function implementations ---------- //
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

// 受信用
void *handle_receive(void *arg) {
  int *sock = (int *) arg;
  char buffer[BUFSIZE];
  while (true) {
    // 受信
    memset(buffer, '\0', BUFSIZE);
    recv(*sock, buffer, BUFSIZE, 0);

    // ユーザー名を下線付きで表示
    printf("\r%s%s%s\n", FONT_UNDERLINED, decode_username(buffer), FONT_RESET);
    // メッセージを表示
    printf(">> %s\n", decode_message(buffer));
    // プロンプトを表示
    printf("\n\r> ");
    fflush(stdout);

    // 終了判定
    if (is_equal_str(decode_message(buffer), "quit")) {
      printf("%sinfo%s quit\n", FONT_CYAN, FONT_RESET);
      break;
    }
  }
  pthread_exit(NULL);
}