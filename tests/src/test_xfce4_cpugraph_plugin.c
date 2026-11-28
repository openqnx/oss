/* xfce4-cpugraph-plugin.
 *  - the module is installed where the panel looks for plugins, its
 *    .desktop file names it, and it loads the way the panel loads a plugin
 *    (GModule) and exports the entry point of an external plugin;
 *  - the source of its numbers on QNX: the plugin computes a processor's
 *    load from the CPU time of that processor's idle thread in the kernel
 *    process (pid 1, thread ids 1..num_cpu) against the monotonic clock.
 *    The test reads the same clocks the same way: every processor has such
 *    a clock, it never runs backwards, the load over an interval is between
 *    0 and 1, and while the test keeps one thread busy for a second the
 *    loads of all processors add up to at least half a processor -- i.e.
 *    the numbers the graph would draw follow what the system is doing. */
#include "common.h"
#include <gmodule.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#ifdef __QNXNTO__
#include <sys/neutrino.h>
#include <sys/syspage.h>
#endif

#define MAXCPU 64

static uint64_t mono_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

/* Idle time of processor cpu (0-based) in ns; 0 and *ok = 0 when unreadable. */
static uint64_t idle_ns(unsigned cpu, int *ok) {
#ifdef __QNXNTO__
  uint64_t v = 0;
  int id = ClockId(1, (int)cpu + 1);
  if (id == -1 || ClockTime(id, NULL, &v) == -1) { *ok = 0; return 0; }
  return v;
#else
  (void)cpu; *ok = 0; return 0;
#endif
}

static volatile int spin_stop;
static void *spin(void *arg) {
  volatile unsigned long x = 0;
  (void)arg;
  while (!spin_stop) x++;
  return NULL;
}

/* Loads of all processors over the next `ms` milliseconds; returns their sum,
 * -1 when a clock could not be read. *range_ok: every load within [0, 1.05]. */
static double measure(unsigned ncpu, int ms, int *range_ok, int *mono_ok) {
  uint64_t i0[MAXCPU], t0, t1;
  int ok = 1;
  double sum = 0;
  for (unsigned c = 0; c < ncpu; c++) i0[c] = idle_ns(c, &ok);
  t0 = mono_ns();
  usleep((useconds_t)ms * 1000);
  t1 = mono_ns();
  for (unsigned c = 0; c < ncpu; c++) {
    uint64_t i1 = idle_ns(c, &ok);
    if (i1 < i0[c]) { *mono_ok = 0; continue; }
    double load = 1.0 - (double)(i1 - i0[c]) / (double)(t1 - t0);
    printf("  info cpu%u: idle %.3f s of %.3f s, load %.2f\n", c, (double)(i1 - i0[c]) / 1e9, (double)(t1 - t0) / 1e9, load);
    if (load < -0.05 || load > 1.05) *range_ok = 0;
    sum += load;
  }
  return ok ? sum : -1;
}

int main(void) {
  QT_BEGIN("xfce4-cpugraph-plugin");
  const char *prefix = getenv("QNX_PORT_PREFIX");
  if (!prefix) prefix = "/usr/local";
  char path[512], line[256];

  /* --- installed, named, loadable --- */
  snprintf(path, sizeof path, "%s/share/xfce4/panel/plugins/cpugraph.desktop", prefix);
  FILE *f = fopen(path, "r");
  int named = 0;
  if (f) {
    while (fgets(line, sizeof line, f)) if (strncmp(line, "X-XFCE-Module=cpugraph", 22) == 0) named = 1;
    fclose(f);
  }
  QT_CHECK(f != NULL, "cpugraph.desktop installed in share/xfce4/panel/plugins");
  QT_CHECK(named, "cpugraph.desktop names the module (X-XFCE-Module=cpugraph)");

  snprintf(path, sizeof path, "%s/lib/xfce4/panel/plugins/libcpugraph.so", prefix);
  GModule *m = g_module_open(path, G_MODULE_BIND_LOCAL);
  if (!m) printf("  info g_module_open: %s\n", g_module_error());
  QT_CHECK(m != NULL, "libcpugraph.so loads (every dependency resolved)");
  if (m) {
    gpointer sym = NULL;
    int has = g_module_symbol(m, "xfce_panel_module_construct", &sym) || g_module_symbol(m, "xfce_panel_module_init", &sym);
    QT_CHECK(has && sym != NULL, "the module exports the panel plugin entry point");
  }

  /* --- the numbers behind the graph --- */
  unsigned ncpu = 0;
#ifdef __QNXNTO__
  ncpu = _syspage_ptr->num_cpu;
#endif
  printf("  info processors: %u\n", ncpu);
  QT_CHECK(ncpu >= 1 && ncpu <= MAXCPU, "processor count from the system page");
  if (ncpu >= 1 && ncpu <= MAXCPU) {
    int ok = 1;
    for (unsigned c = 0; c < ncpu; c++) (void)idle_ns(c, &ok);
    QT_CHECK(ok, "every processor's idle thread (pid 1, tid cpu+1) has a readable CPU-time clock");

    int range_ok = 1, mono_ok = 1;
    double quiet = measure(ncpu, 1000, &range_ok, &mono_ok);
    printf("  info at rest: loads add up to %.2f\n", quiet);

    pthread_t th;
    spin_stop = 0;
    int started = pthread_create(&th, NULL, spin, NULL) == 0;
    double busy = measure(ncpu, 1000, &range_ok, &mono_ok);
    spin_stop = 1;
    if (started) pthread_join(th, NULL);
    printf("  info with one busy thread: loads add up to %.2f\n", busy);

    QT_CHECK(mono_ok, "idle time never runs backwards");
    QT_CHECK(quiet >= 0 && busy >= 0 && range_ok, "every processor's load is between 0 and 1");
    QT_CHECK(started && busy >= 0.5, "one busy thread shows as at least half a processor of load");
  }
  QT_END();
}
