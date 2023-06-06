#pragma once

#include <stdlib.h>
#include <string.h>

#include "string.h"

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
void sanitize(string* res, string* str) {
  struct string__node* p = str->head;
  while (p != NULL) {
    if (p->val == '\n') {
      string__push_back(res, ' ');
    }
    else {
      string__push_back(res, p->val);
    }
    p = p->next;
  }
}

// 名前とメッセージを分離
void decode_username(string** res, string* encoded) {
  string__init(res);
  struct string__node* p = encoded->head;
  while (p != NULL) {
    if (p->val == '\n') {
      break;
    }
    string__push_back(*res, p->val);
    p = p->next;
  }
}

void decode_message(string** res, string* encoded) {
  string__init(res);
  struct string__node* p = encoded->head;
  bool flag = false;
  while (p != NULL) {
    if (flag) string__push_back(*res, p->val);
    if (p->val == '\n') flag = true;
    p = p->next;
  }
}