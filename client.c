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
#include "sock.h"

int main(int argc, char *argv[]) {
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

  struct sockaddr_in server;
  memset((void *) &server, 0, sizeof(server));
  server.sin_family = PF_INET;
  server.sin_port = htons(port);
  memcpy((void *) &server.sin_addr, (void *) hostent->h_addr, hostent->h_length);

  int sock;
  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    printf("%serror%s socket creation failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }
  if (connect(sock, (struct sockaddr *) &server, sizeof(server)) == -1) {
    printf("%serror%s connect failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  printf("%sinfo%s connected\n", FONT_CYAN, FONT_RESET);

  printf("%stips%s send %s/help%s to show how to use\n", FONT_PURPLE, FONT_RESET, FONT_BOLD, FONT_RESET);
  printf("\n\r> ");
  fflush(stdout);

  // create thread
  pthread_t send_thread, receive_thread;
  if (pthread_create(&send_thread, NULL, handle_send, (void *) &sock) != 0) {
    printf("%serror%s sender pthread_create failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }
  if (pthread_create(&receive_thread, NULL, handle_receive, (void *) &sock) != 0) {
    printf("%serror%s receiver pthread_create failed\n", FONT_RED, FONT_RESET);
    exit(EXIT_FAILURE);
  }

  // watch thread
  bool sender_terminated = false, receiver_terminated = false;
  while (true) {
    if (sender_terminated && receiver_terminated) break;

    // sender
    if (pthread_tryjoin_np(send_thread, NULL) == 0) {
      sender_terminated = true;
      printf("%ssuccess%s sender pthread_join was terminated\n", FONT_GREEN, FONT_RESET);
      if (!receiver_terminated) {
        printf("%sinfo%s trying to terminate receiver thread\n", FONT_CYAN, FONT_RESET);
        if (pthread_cancel(receive_thread) != 0) printf("%serror%s receiver pthread_cancel failed\n", FONT_RED, FONT_RESET);
      }
    }
    // receiver
    if (pthread_tryjoin_np(receive_thread, NULL) == 0) {
      receiver_terminated = true;
      printf("%ssuccess%s receiver pthread_join success\n", FONT_GREEN, FONT_RESET);
      if (!sender_terminated) {
        printf("%sinfo%s trying to terminate sender thread\n", FONT_CYAN, FONT_RESET);
        if (pthread_cancel(send_thread) != 0) printf("%serror%s sender pthread_cancel failed\n", FONT_RED, FONT_RESET);
      }
    }
  }
  close(sock);
  exit(EXIT_SUCCESS);
}