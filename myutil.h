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
