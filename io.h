#pragma once

#include <string.h>

const char* FONT_RED = "\033[31m";
const char* FONT_GREEN = "\033[32m";
const char* FONT_YELLOW = "\033[33m";
const char* FONT_BLUE = "\033[34m";
const char* FONT_PURPLE = "\033[35m";
const char* FONT_CYAN = "\033[36m";
const char* FONT_UNDERLINED = "\033[4m";
const char* FONT_BOLD = "\033[1m";
const char* FONT_RESET = "\033[0m";

const int BUFSIZE = 1024;

void chop(char* str) {
  char* p = strchr(str, '\n');
  if (p != NULL) *p = '\0';
}

char* sanitize(char* str) {
  char* ret = (char*) malloc(BUFSIZE);
  memset(ret, '\0', BUFSIZE);
  for (int i = 0; i < strlen(str); i++) {
    if (str[i] == '\n') ret[i] = ' ';
    ret[i] = str[i];
  }
  return ret;
}

char* encode(char* username, char* message) {
  char* encoded = (char*) malloc(BUFSIZE);
  memset(encoded, '\0', BUFSIZE);
  strcat(encoded, sanitize(username));
  strcat(encoded, "\n");
  strcat(encoded, sanitize(message));
  return encoded;
}

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