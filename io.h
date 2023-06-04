#pragma once
#include <string.h>

// フォント
const char* FONT_RED = "\033[31m";
const char* FONT_GREEN = "\033[32m";
const char* FONT_YELLOW = "\033[33m";
const char* FONT_BLUE = "\033[34m";
const char* FONT_PURPLE = "\033[35m";
const char* FONT_CYAN = "\033[36m";
const char* FONT_UNDERLINED = "\033[4m";
const char* FONT_BOLD = "\033[1m";
const char* FONT_RESET = "\033[0m";

// Buffer サイズ
#define BUFSIZE 1024

// 改行コード削除
void chop(char* str) {
  char* p = strchr(str, '\n');
  if (p != NULL) *p = '\0';
}

// 一応サニタイズ
char* sanitize(char* str) {
  char* ret = (char*) malloc(BUFSIZE);
  memset(ret, '\0', BUFSIZE);
  for (int i = 0; i < strlen(str); i++) {
    if (str[i] == '\n') ret[i] = ' ';
    ret[i] = str[i];
  }
  return ret;
}

// 名前とメッセージを分離
char* decode_username(char* encoded) {
  char* username = (char*) malloc(BUFSIZE);
  memset(username, '\0', BUFSIZE);
  char* p = strchr(encoded, '\n');
  if (p != NULL) {
    strncpy(username, encoded, p - encoded);
  }
  return username;
}
char* decode_message(char* encoded) {
  char* message = (char*) malloc(BUFSIZE);
  memset(message, '\0', BUFSIZE);
  char* p = strchr(encoded, '\n');
  if (p != NULL) {
    strcpy(message, p + 1);
  }
  return message;
}