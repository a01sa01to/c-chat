#include <netdb.h>
#include <unistd.h>

#include "client_h/receive.h"
#include "client_h/send.h"
#include "common/io.h"
#include "common/myutil.h"

// ---------- main function ---------- //
int main(int argc, char *argv[]) {
  // まずはコマンドが正しく入力されているかを確認する
  if (argc != 3) {
    printf("%serror%s invalid number of arguments\n", FONT_RED, FONT_RESET);
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
      // 受信側を終了させる
      if (!receiver_terminated) {
        if (pthread_cancel(receive_thread) != 0) printf("%serror%s receiver thread cancellation failed\n", FONT_RED, FONT_RESET);
      }
    }

    // 受信側
    if (pthread_tryjoin_np(receive_thread, NULL) == 0) {
      receiver_terminated = true;
      // 送信側を終了させる
      if (!sender_terminated) {
        if (pthread_cancel(send_thread) != 0) printf("%serror%s sender thread cancellation failed\n", FONT_RED, FONT_RESET);
      }
    }
  }
  close(sock);
  exit(EXIT_SUCCESS);
}
