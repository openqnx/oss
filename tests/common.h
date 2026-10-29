/* Minimal PASS/FAIL harness shared by every port test.
 * Each test is a standalone QNX binary: it exercises the library's core API,
 * prints one line per check, and exits 0 only if every check passed. */
#ifndef QNX_PORT_TEST_COMMON_H
#define QNX_PORT_TEST_COMMON_H
#include <stdio.h>
#include <string.h>
static int qt_fail_count = 0;
static const char *qt_name = "test";
/* Unbuffered: when a library aborts mid-test, a buffered stdout takes every
   check printed so far with it, and the target shows nothing but "Abort". */
#define QT_BEGIN(n) do { setvbuf(stdout, NULL, _IONBF, 0); qt_name = (n); \
                         printf("== %s ==\n", qt_name); } while (0)
#define QT_CHECK(cond, what) do {                                             \
    if (cond) { printf("  ok   %s\n", (what)); }                              \
    else { printf("  FAIL %s  (%s:%d)\n", (what), __FILE__, __LINE__); ++qt_fail_count; } \
  } while (0)
#define QT_END() do {                                                         \
    if (qt_fail_count == 0) { printf("PASS %s\n", qt_name); return 0; }       \
    printf("FAIL %s (%d check(s) failed)\n", qt_name, qt_fail_count);         \
    return 1;                                                                 \
  } while (0)
#endif
