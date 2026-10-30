#include "common.h"
#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#define UNW_LOCAL_ONLY
#include <libunwind.h>
/* Unwind a known call chain, main -> level1 -> level2 -> level3, from
 * inside level3 with the libunwind of /usr/local/opt/libunwind (upstream
 * 1.8.3, not the SDP's 1.8.2): every frame named, in order; unw_backtrace
 * gives the same depth; the library that answered is the one in opt/. */
static char names[16][64]; static int depth;
static void __attribute__((noinline)) level3(void) {
  unw_context_t uc; unw_cursor_t cur;
  QT_CHECK(unw_getcontext(&uc) == 0, "unw_getcontext");
  QT_CHECK(unw_init_local(&cur, &uc) == 0, "unw_init_local");
  depth = 0;
  do {
    unw_word_t off; char n[64] = "";
    if (unw_get_proc_name(&cur, n, sizeof n, &off) != 0) strcpy(n, "?");
    snprintf(names[depth], 64, "%s", n);
    printf("  info frame %d: %s+0x%lx\n", depth, n, (unsigned long)off);
  } while (++depth < 16 && unw_step(&cur) > 0);
  void *bt[16]; int n = unw_backtrace(bt, 16);
  printf("  info unw_backtrace: %d frames\n", n);
  QT_CHECK(n >= 4, "unw_backtrace saw the call chain");
}
static void __attribute__((noinline)) level2(void) { level3(); __asm__ volatile(""); }
static void __attribute__((noinline)) level1(void) { level2(); __asm__ volatile(""); }

int main(void) {
  QT_BEGIN("libunwind");
  level1();
  const char *want[] = { "level3", "level2", "level1", "main" };
  for (int i = 0; i < 4; i++) {
    char what[64]; snprintf(what, sizeof what, "frame %d is %s", i, want[i]);
    QT_CHECK(i < depth && strcmp(names[i], want[i]) == 0, what);
  }
  Dl_info di;
  if (dladdr((void *)unw_backtrace, &di) && di.dli_fname) {
    printf("  info unw_backtrace from %s\n", di.dli_fname);
    QT_CHECK(strstr(di.dli_fname, "opt/libunwind") != NULL, "the opt/libunwind library answered");
  } else QT_CHECK(0, "dladdr(unw_backtrace)");
  printf("  info headers: libunwind %d.%d.%d\n", UNW_VERSION_MAJOR, UNW_VERSION_MINOR, UNW_VERSION_EXTRA);
  QT_CHECK(UNW_VERSION_MAJOR == 1 && UNW_VERSION_MINOR == 8 && UNW_VERSION_EXTRA == 3, "built against 1.8.3");
  QT_END();
}
