#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memset() */

#define mleak_malloc(n) mleak_malloc_helper(n, __FILE__, __LINE__)
#define mleak_free(p) mleak_free_helper(p, __FILE__, __LINE__)

void *mleak_malloc_helper(size_t size, const char *filename, int linenumber);
void mleak_free_helper(void *p, const char *filename, int linenumber);
void mleak_finalize(void);

/* 配列超過のチェック用マジックナンバー */
#define MLEAK_SIGNATURE_0 0x94a3ec07
#define MLEAK_SIGNATURE_1 0x5bf128d6

/* mleak が管理するスペースの基本単位を表す共用体 */
typedef union {
  struct {
    void *next;
    size_t size;
  } header;
  struct {
    const char *filename;
    int linenumber;
  } footer;
  unsigned int signature[2];
} mleak_piece;

/* mleak が管理するスペースを指すポインタ */
static mleak_piece *head = NULL;

/* malloc のラッパ関数。この関数を直接呼ばずに mleak_malloc() を呼ぶこと */
void *mleak_malloc_helper(size_t size, const char *filename, int linenumber) {
  size_t n = (size + sizeof(mleak_piece) - 1) / sizeof(mleak_piece) + 3;
  mleak_piece *p = (mleak_piece *) malloc(sizeof(mleak_piece) * n);
  p[0].header.next = head;
  p[0].header.size = n;
  p[n - 2].signature[0] = MLEAK_SIGNATURE_0;
  p[n - 2].signature[1] = MLEAK_SIGNATURE_1;
  p[n - 1].footer.filename = filename;
  p[n - 1].footer.linenumber = linenumber;
  head = p;
  return p + 1;
}

/* free のラッパ関数。この関数を直接呼ばずに mleak_free() を呼ぶこと */
void mleak_free_helper(void *p, const char *filename, int linenumber) {
  mleak_piece *prev = NULL, *q;
  size_t n;
  void *freed;
  for (q = head; q != NULL; q = q->header.next) {
    if (q + 1 == p) {
      if (prev != NULL) {
        prev->header.next = q->header.next;
      }
      else {
        head = q->header.next;
      }
      n = q->header.size;
      if (q[n - 2].signature[0] != MLEAK_SIGNATURE_0 || q[n - 2].signature[1] != MLEAK_SIGNATURE_1) {
        fprintf(stderr,
                "mleak_free: error: (free @ %s, %d): "
                "適切に解放できませんでした。確保したスペースを超過して使用したおそれがあります。\n",
                filename, linenumber);
      }
      /*
				fprintf(stderr, "mleak_free: (allocate @ %s, %d; free @ %s, %d): 適切に解放できました。\n",
					q[n-1].footer.filename, q[n-1].footer.linenumber, filename, linenumber);
			*/
      /* memset(q, 0x55, sizeof(mleak_piece) * n); */
      memset(q + 1, 0x55, sizeof(mleak_piece) * (n - 2));
      free(q);
      return;
    }
    prev = q;
  }
  memset(&freed, 0x55, sizeof(void *));
  if (p != freed) {
    fprintf(stderr,
            "mleak_free: error: (free @ %s, %d): "
            "適切に解放できませんでした。二重解放のおそれがあります。\n",
            filename, linenumber);
  }
  else {
    fprintf(stderr,
            "mleak_free: error: (free @ %s, %d): "
            "適切に解放できませんでした。既に解放した領域を使用しているおそれがあります。\n",
            filename, linenumber);
  }
}

/* mleak の後処理関数。atexit() に登録しておくと簡単 */
void mleak_finalize(void) {
  mleak_piece *p, *q;
  size_t n;

  if (head == NULL) {
    fprintf(stderr, "mleak_finalize: info: 正常にプログラムが終了しました。\n");
    return;
  }
  for (p = head; p != NULL; p = q) {
    n = p->header.size;
    fprintf(stderr, "mleak_finalize: error: (allocate @ %s, %d): プログラム終了まで解放されませんでした。\n",
            p[n - 1].footer.filename, p[n - 1].footer.linenumber);
    q = p->header.next;
    free(p);
  }
}

#define malloc mleak_malloc
#define free mleak_free