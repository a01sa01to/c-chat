#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myutil.h"

// ---------- set ---------- //
// 二分探索木を set<string> の代わりに使う
typedef struct {
  char* key;
  set_str__node* left;
  set_str__node* right;
} set_str__node;

typedef struct {
  set_str__node* root;
} set_str;

set_str* set_str__new() {
  set_str* h = (set_str*) malloc(sizeof(set_str));
  h->root = NULL;
  return h;
}

void set_str__insert(set_str* h, char* key) {
  set_str__node* n = (set_str__node*) malloc(sizeof(set_str__node));
  n->key = key;
  n->left = n->right = NULL;
  if (h->root == NULL) {
    h->root = n;
    return;
  }
  set_str__node* p = h->root;
  while (true) {
    if (is_greater_str(key, p->key)) {
      if (p->right == NULL) {
        p->right = n;
        return;
      }
      p = p->right;
    }
    else if (is_less_str(key, p->key)) {
      if (p->left == NULL) {
        p->left = n;
        return;
      }
      p = p->left;
    }
    else {
      return;
    }
  }
}

void set_str__erase(set_str* h, char* key) {
  if (h->root == NULL) {
    return;
  }
  set_str__node* p = h->root;
  set_str__node* pp = NULL;
  while (true) {
    if (is_greater_str(key, p->key)) {
      if (p->right == NULL) {
        return;
      }
      pp = p;
      p = p->right;
    }
    else if (is_less_str(key, p->key)) {
      if (p->left == NULL) {
        return;
      }
      pp = p;
      p = p->left;
    }
    else {
      break;
    }
  }
  if (p->left == NULL && p->right == NULL && pp == NULL) {
    h->root = NULL;
    return;
  }
  if (pp->left == p) {
    if (p->left != NULL) {
      pp->left = p->left;
    }
    else {
      pp->left = p->right;
    }
  }
  else {
    if (p->left != NULL) {
      pp->right = p->left;
    }
    else {
      pp->right = p->right;
    }
  }
}

bool set_str__contains(set_str* h, char* key) {
  if (h->root == NULL) {
    return false;
  }
  set_str__node* p = h->root;
  while (true) {
    if (is_greater_str(key, p->key)) {
      if (p->right == NULL) {
        return false;
      }
      p = p->right;
    }
    else if (is_less_str(key, p->key)) {
      if (p->left == NULL) {
        return false;
      }
      p = p->left;
    }
    else {
      return true;
    }
  }
}