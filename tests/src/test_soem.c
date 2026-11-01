/* SOEM (Simple Open EtherCAT Master): what can be checked without EtherCAT
 * hardware. ecx_init() on a network interface that does not exist fails (the
 * QNX NIC layer opens it through BPF/libpcap) and the context closes
 * cleanly; slaveinfo run on that interface reports "No socket connection"
 * and slaveinfo without arguments lists the adapters, as does
 * ec_find_adapters() (the loopback interface is among them). The OS layer:
 * the monotonic clock advances, osal_usleep sleeps at least as long as
 * asked, timers expire on time, osal_monotonic_sleep wakes at an absolute
 * time, time differences are normalised, mutexes and threads work; byte
 * order helpers, the EtherCAT ethertype in a frame header built by
 * ec_setupheader(), and the AL status / SDO abort code texts. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include "soem/soem.h"

static double secs(const ec_timet *t) { return t->tv_sec + t->tv_nsec / 1e9; }
static void *mtx; static volatile int counter;
static void worker(void *arg) {
  for (int i = 0; i < 10000; i++) { osal_mutex_lock(mtx); counter++; osal_mutex_unlock(mtx); }
  *(volatile int *)arg = 1;
}
static char out[8192];
static int run(const char *cmd) {
  out[0] = 0; FILE *p = popen(cmd, "r"); if (!p) return -1;
  size_t n = fread(out, 1, sizeof out - 1, p); out[n] = 0;
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

int main(void) {
  QT_BEGIN("soem");
  static ecx_contextt ctx;
  QT_CHECK(ecx_init(&ctx, "qnxnoeth9") == 0, "ecx_init on a nonexistent interface fails");
  ecx_close(&ctx);
  QT_CHECK(1, "ecx_close after the failed init");

  ec_adaptert *head = ec_find_adapters(), *a; int n = 0, lo = 0;
  for (a = head; a; a = a->next) { n++; if (!strncmp(a->name, "lo", 2)) lo = 1; printf("  info adapter %s\n", a->name); }
  ec_free_adapters(head);
  QT_CHECK(n > 0 && lo, "ec_find_adapters lists the interfaces, loopback among them");

  int rc = run("{ /usr/local/bin/slaveinfo qnxnoeth9 || slaveinfo qnxnoeth9; } 2>&1");
  QT_CHECK(rc == 0 && strstr(out, "No socket connection on qnxnoeth9"), "slaveinfo on a nonexistent interface: No socket connection");
  rc = run("{ /usr/local/bin/slaveinfo || slaveinfo; } 2>&1");
  QT_CHECK(rc == 0 && strstr(out, "Available adapters:") && strstr(out, "    - lo"), "slaveinfo without arguments lists the adapters");

  /* OS abstraction */
  ec_timet t0, t1, d;
  osal_get_monotonic_time(&t0);
  QT_CHECK(osal_usleep(20000) == 0, "osal_usleep(20 ms) returns 0");
  osal_get_monotonic_time(&t1);
  osal_time_diff(&t0, &t1, &d);
  printf("  info osal_usleep(20000) took %.2f ms\n", secs(&d) * 1e3);
  QT_CHECK(secs(&d) >= 0.020 && secs(&d) < 1.0, "monotonic clock: at least 20 ms elapsed");
  QT_CHECK(d.tv_nsec >= 0 && d.tv_nsec < 1000000000, "time difference normalised");
  ec_timet a1 = { 5, 900000000 }, a2 = { 7, 100000000 }, dd;
  osal_time_diff(&a1, &a2, &dd);
  QT_CHECK(dd.tv_sec == 1 && dd.tv_nsec == 200000000, "osal_time_diff borrows across seconds (1.2 s)");

  osal_timert tm;
  osal_timer_start(&tm, 50000);
  QT_CHECK(!osal_timer_is_expired(&tm), "a 50 ms timer has not expired at once");
  osal_usleep(70000);
  QT_CHECK(osal_timer_is_expired(&tm), "the timer has expired after 70 ms");

  ec_timet wake; osal_get_monotonic_time(&wake);
  wake.tv_nsec += 30000000; if (wake.tv_nsec >= 1000000000) { wake.tv_sec++; wake.tv_nsec -= 1000000000; }
  QT_CHECK(osal_monotonic_sleep(&wake) == 0, "osal_monotonic_sleep to an absolute time");
  osal_get_monotonic_time(&t1);
  QT_CHECK(t1.tv_sec > wake.tv_sec || (t1.tv_sec == wake.tv_sec && t1.tv_nsec >= wake.tv_nsec), "woke at or after the requested time");
  ec_timet rt = osal_current_time();
  QT_CHECK(rt.tv_sec > 1600000000, "osal_current_time is wall-clock time");

  mtx = osal_mutex_create();
  volatile int done1 = 0, done2 = 0; pthread_t th1, th2;
  QT_CHECK(mtx && osal_thread_create(&th1, 128 * 1024, worker, (void *)&done1) && osal_thread_create(&th2, 128 * 1024, worker, (void *)&done2),
           "osal_thread_create (two threads)");
  pthread_join(th1, NULL); pthread_join(th2, NULL);
  QT_CHECK(done1 && done2 && counter == 20000, "osal mutex: 2 x 10000 increments, none lost");
  osal_mutex_destroy(mtx);
  void *p = osal_malloc(1000); QT_CHECK(p != NULL, "osal_malloc"); osal_free(p);

  /* byte order and frame layout */
  QT_CHECK(oshw_htons(0x1234) == htons(0x1234) && oshw_ntohs(htons(0xBEEF)) == 0xBEEF, "oshw_htons/oshw_ntohs");
  ec_bufT frame; memset(frame, 0, sizeof frame);
  ec_setupheader(&frame);
  ec_etherheadert *eh = (ec_etherheadert *)frame;
  QT_CHECK(ntohs(eh->etype) == ETH_P_ECAT && ETH_P_ECAT == 0x88A4, "ec_setupheader: ethertype 0x88A4");
  QT_CHECK(eh->da0 == 0xffff && eh->da1 == 0xffff && eh->da2 == 0xffff, "ec_setupheader: broadcast destination");

  QT_CHECK(!strcmp(ec_ALstatuscode2string(0x0011), "Invalid requested state change"), "AL status code 0x0011 text");
  QT_CHECK(!strcmp(ec_sdoerror2string(0x06020000), "The object does not exist in the object directory"), "SDO abort code 0x06020000 text");
  QT_END();
}
