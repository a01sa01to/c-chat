#pragma once

#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>

#include "../common/io.h"
#include "../common/string.h"
#define NAME_LEN 20

typedef struct {
  string* content;
  string* sender_name;
  int sender_id;
  int message_id;
} message_state;

typedef struct {
  struct sockaddr_in addr;
  int sock;
  string* name;
  pthread_t send_thread, recv_thread;
  int id;
  int last_message_id;
  bool send_created, recv_created;
  bool send_terminated, recv_terminated;
  message_state* msg;
} client_t;
