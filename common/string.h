#pragma once

#include <stdbool.h>
#include <string.h>

#include "mleak.h"

struct string__node {
  char val;
  struct string__node *next;
};

typedef struct {
  struct string__node *head;
  int length;
} string;

void string__free(string *s) {
  struct string__node *p = s->head;
  while (p != NULL) {
    struct string__node *q = p->next;
    free(p);
    p = q;
  }
  free(s);
}

void string__init(string *s) {
  if (s == NULL) {
    puts("error: string__init s is NULL");
    exit(EXIT_FAILURE);
  }
  s->head = NULL;
  s->length = 0;
}

void string__push_back(string *s, char c) {
  struct string__node *node = (struct string__node *) malloc(sizeof(struct string__node));
  node->val = c;
  node->next = NULL;
  s->length++;
  if (s->head == NULL) {
    s->head = node;
  }
  else {
    struct string__node *p = s->head;
    while (p->next != NULL) p = p->next;
    p->next = node;
  }
}

void string__from_cstr(string *s, char *cstr) {
  string__init(s);
  int len = strlen(cstr);
  for (int i = 0; i < len; i++) string__push_back(s, cstr[i]);
}

void string__append(string *s, string *t) {
  struct string__node *p = t->head;
  while (p != NULL) {
    string__push_back(s, p->val);
    p = p->next;
  }
}

void string__concat(string *res, string *lhs, string *rhs) {
  string__init(res);
  struct string__node *p = lhs->head;
  while (p != NULL) {
    string__push_back(res, p->val);
    p = p->next;
  }
  p = rhs->head;
  while (p != NULL) {
    string__push_back(res, p->val);
    p = p->next;
  }
}

void string__copy(string *res, string *s) {
  string__init(res);
  struct string__node *p = s->head;
  while (p != NULL) {
    string__push_back(res, p->val);
    p = p->next;
  }
}

bool string__startsWith(string *s, string *prefix) {
  struct string__node *p = s->head, *q = prefix->head;
  while (p != NULL && q != NULL) {
    if (p->val != q->val) return false;
    p = p->next, q = q->next;
  }
  if (q != NULL) return false;
  return true;
}

void string2cstr(string *s, char *res) {
  struct string__node *p = s->head;
  int i = 0;
  while (p != NULL) {
    res[i++] = p->val;
    p = p->next;
  }
  res[i] = '\0';
}

int strcmp_impl(string *lhs, string *rhs) {
  struct string__node *p = lhs->head, *q = rhs->head;
  while (p != NULL && q != NULL) {
    if (p->val < q->val) return -1;
    if (p->val > q->val) return 1;
    p = p->next, q = q->next;
  }
  if (p == NULL && q == NULL) return 0;
  if (p == NULL) return -1;
  return 1;
}

bool is_greater_str(string *lhs, string *rhs) {
  return strcmp_impl(lhs, rhs) > 0;
}
bool is_less_str(string *lhs, string *rhs) {
  return strcmp_impl(lhs, rhs) < 0;
}
bool is_equal_str(string *lhs, string *rhs) {
  return strcmp_impl(lhs, rhs) == 0;
}