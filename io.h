#pragma once

#include <string.h>

const char* FONT_RED = "\033[31m";
const char* FONT_GREEN = "\033[32m";
const char* FONT_BROWN = "\033[33m";
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