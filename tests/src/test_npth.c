/* npth (the New GNU Portable Threads library): npth_init, threads that only
 * run while they hold the global lock -- a counter incremented by 4 threads
 * 1000 times each without a mutex must come out at exactly 4000 when every
 * increment yields (npth_usleep) only outside the read-modify-write --,
 * thread return values through npth_join, a mutex/condition handshake,
 * npth_sleep timing, rwlocks, and npth_select/npth_read on a pipe. */
#include "common.h"
#include <npth.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NT 4
#define NI 1000
static long counter;

static void *worker(void *arg) {
  intptr_t id = (intptr_t)arg;
  for (int i = 0; i < NI; i++) {
    long c = counter;     /* only one npth thread runs at a time ... */
    counter = c + 1;
    if (i % 100 == 0) npth_usleep(100);  /* ... and switches only here */
  }
  return (void *)(id * 10);
}

static npth_mutex_t m = NPTH_MUTEX_INITIALIZER;
static npth_cond_t cv = NPTH_COND_INITIALIZER;
static int ready, answer;

static void *signaller(void *arg) {
  (void)arg;
  npth_usleep(20000);
  npth_mutex_lock(&m);
  answer = 42; ready = 1;
  npth_cond_signal(&cv);
  npth_mutex_unlock(&m);
  return NULL;
}

static int pfd[2];
static void *writer(void *arg) {
  (void)arg;
  npth_usleep(50000);
  npth_write(pfd[1], "npth", 4);
  return NULL;
}

static double now(void) {
  struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(void) {
  QT_BEGIN("npth");
  QT_CHECK(npth_init() == 0, "npth_init");

  npth_t th[NT];
  int ok = 1;
  for (intptr_t i = 0; i < NT; i++) ok &= npth_create(&th[i], NULL, worker, (void *)(i + 1)) == 0;
  QT_CHECK(ok, "4 threads created");
  long sum = 0;
  for (int i = 0; i < NT; i++) { void *r = NULL; ok &= npth_join(th[i], &r) == 0; sum += (long)(intptr_t)r; }
  QT_CHECK(ok && sum == 100, "join returns each thread's value (10+20+30+40 = 100)");
  printf("  info counter = %ld\n", counter);
  QT_CHECK(counter == NT * NI, "unlocked counter is exactly 4000 (one thread runs at a time)");

  npth_t s;
  npth_create(&s, NULL, signaller, NULL);
  npth_mutex_lock(&m);
  while (!ready) npth_cond_wait(&cv, &m);
  npth_mutex_unlock(&m);
  npth_join(s, NULL);
  QT_CHECK(answer == 42, "condition variable handshake delivers 42");

  struct timespec abst;
  npth_clock_gettime(&abst);
  abst.tv_nsec += 100 * 1000000L;
  if (abst.tv_nsec >= 1000000000L) { abst.tv_sec++; abst.tv_nsec -= 1000000000L; }
  npth_mutex_lock(&m);
  int rc = npth_cond_timedwait(&cv, &m, &abst);
  npth_mutex_unlock(&m);
  QT_CHECK(rc == ETIMEDOUT, "cond_timedwait with nobody signalling times out (ETIMEDOUT)");

  double t0 = now();
  npth_sleep(1);
  double dt = now() - t0;
  printf("  info npth_sleep(1) took %.3f s\n", dt);
  QT_CHECK(dt >= 0.95 && dt < 3.0, "npth_sleep(1) sleeps about one second");

  npth_rwlock_t rw;
  QT_CHECK(npth_rwlock_init(&rw, NULL) == 0 && npth_rwlock_rdlock(&rw) == 0 && npth_rwlock_tryrdlock(&rw) == 0,
           "rwlock: two read locks at once");
  rc = npth_rwlock_trywrlock(&rw);
  printf("  info trywrlock while read-locked: %d\n", rc);
  QT_CHECK(rc != 0, "rwlock: write lock refused while read-locked");
  npth_rwlock_unlock(&rw); npth_rwlock_unlock(&rw);
  QT_CHECK(npth_rwlock_trywrlock(&rw) == 0, "rwlock: write lock granted once released");
  npth_rwlock_unlock(&rw);

  QT_CHECK(pipe(pfd) == 0, "pipe");
  npth_t w;
  npth_create(&w, NULL, writer, NULL);
  fd_set rf; FD_ZERO(&rf); FD_SET(pfd[0], &rf);
  struct timeval tv = { 5, 0 };
  int n = npth_select(pfd[0] + 1, &rf, NULL, NULL, &tv);
  char buf[8] = {0};
  QT_CHECK(n == 1 && FD_ISSET(pfd[0], &rf), "npth_select wakes when another thread writes");
  QT_CHECK(npth_read(pfd[0], buf, 4) == 4 && memcmp(buf, "npth", 4) == 0, "npth_read gets \"npth\"");
  npth_join(w, NULL);
  QT_END();
}
