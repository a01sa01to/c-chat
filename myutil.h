#include <stdbool.h>
#include <string.h>

// ---------- string ---------- //
bool is_greater_str(char* lhs, char* rhs) {
  return strcmp(lhs, rhs) > 0;
}
bool is_less_str(char* lhs, char* rhs) {
  return strcmp(lhs, rhs) < 0;
}
bool is_equal_str(char* lhs, char* rhs) {
  return strcmp(lhs, rhs) == 0;
}

// ---------- string to int ----------- //
// invalid: -1
int str2portNum(char* str) {
  int res = 0;
  for (int i = 0; str[i] != '\0'; i++) {
    if (res > 65535) return -1;
    if (str[i] < '0' || str[i] > '9') return -1;
    res = res * 10 + str[i] - '0';
  }
  if (res > 65535) return -1;
  return res;
}