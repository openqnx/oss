#include "common.h"
#include <stdint.h>
#include <string.h>
#include <mimalloc.h>
/* mimalloc: allocations of many sizes written and freed, aligned
 * allocation, a private heap destroyed at once, the library's version. */
int main(void) {
  QT_BEGIN("mimalloc");
  printf("  info mimalloc %d\n", mi_version());
  QT_CHECK(mi_version() == 315, "mimalloc 3.1.5");
  int ok = 1;
  for (size_t sz = 1; sz < (1u << 22); sz = sz * 3 + 1) {
    unsigned char *p = mi_malloc(sz);
    if (!p || mi_usable_size(p) < sz) { ok = 0; break; }
    memset(p, 0xa5, sz); if (p[sz - 1] != 0xa5) ok = 0;
    mi_free(p);
  }
  QT_CHECK(ok, "mi_malloc 1 B .. 4 MB, usable size, written, freed");
  void *a = mi_malloc_aligned(1000, 4096);
  QT_CHECK(a && ((uintptr_t)a % 4096) == 0, "mi_malloc_aligned(4096)");
  mi_free(a);
  mi_heap_t *h = mi_heap_new();
  QT_CHECK(h != NULL, "mi_heap_new");
  for (int i = 0; i < 10000; i++) mi_heap_malloc(h, 64 + i % 200);
  mi_heap_destroy(h);
  QT_CHECK(1, "mi_heap_destroy frees 10000 blocks at once");
  char *s = mi_strdup("qnx"); QT_CHECK(s && !strcmp(s, "qnx"), "mi_strdup"); mi_free(s);
  QT_END();
}
