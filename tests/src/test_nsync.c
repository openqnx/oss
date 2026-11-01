/* nsync: 4 threads x 20000 increments under an nsync_mu; readers sharing
 * the lock (two readers inside at once); a producer/consumer queue on an
 * nsync_cv; a conditional critical section (nsync_mu_wait); cv and
 * counter waits that time out after 100 ms (and not much earlier); an
 * nsync_counter as a barrier for 4 threads; an nsync_note notified by
 * another thread; nsync_run_once from 4 threads running its function once. */
#include "common.h"
#include <nsync.h>
#include <pthread.h>

static nsync_mu mu = NSYNC_MU_INIT;
static nsync_cv cv = NSYNC_CV_INIT;
static long count;
static int queue[16], qn, readers_in, max_readers;
static nsync_counter done;
static nsync_once once = NSYNC_ONCE_INIT;
static int once_runs;
static void once_fn(void) { __sync_fetch_and_add(&once_runs, 1); }

static void *incr(void *a) {
  int i; (void)a;
  for (i = 0; i < 20000; i++) { nsync_mu_lock(&mu); count++; nsync_mu_unlock(&mu); }
  nsync_run_once(&once, once_fn);
  nsync_counter_add(done, -1);
  return NULL;
}
static void *reader(void *a) {
  (void)a;
  nsync_mu_rlock(&mu);
  __sync_fetch_and_add(&readers_in, 1);
  { nsync_time end = nsync_time_add(nsync_time_now(), nsync_time_ms(2000));
    while (__sync_fetch_and_add(&readers_in, 0) < 2 && nsync_time_cmp(nsync_time_now(), end) < 0) nsync_time_sleep(nsync_time_ms(1)); }
  if (readers_in > max_readers) max_readers = readers_in;
  nsync_mu_runlock(&mu);
  return NULL;
}
static void *producer(void *a) {
  int i; (void)a;
  for (i = 1; i <= 100; i++) {
    nsync_mu_lock(&mu);
    while (qn == 16) nsync_cv_wait(&cv, &mu);
    queue[qn++] = i; nsync_cv_broadcast(&cv);
    nsync_mu_unlock(&mu);
  }
  return NULL;
}
static void *count_only(void *a) {
  int i; (void)a;
  for (i = 0; i < 20000; i++) { nsync_mu_lock(&mu); count++; nsync_mu_unlock(&mu); }
  return NULL;
}
static int ge50(const void *arg) { return *(const long *)arg >= 50; }
static void *notifier(void *n) { nsync_time_sleep(nsync_time_ms(50)); nsync_note_notify((nsync_note)n); return NULL; }

int main(void) {
  pthread_t t[4]; int i; long sum = 0; nsync_time t0, el;
  QT_BEGIN("nsync");
  done = nsync_counter_new(4);
  for (i = 0; i < 4; i++) pthread_create(&t[i], NULL, incr, NULL);
  QT_CHECK(nsync_counter_wait(done, nsync_time_add(nsync_time_now(), nsync_time_ms(60000))) == 0, "nsync_counter reaches 0 when 4 threads finish");
  for (i = 0; i < 4; i++) pthread_join(t[i], NULL);
  QT_CHECK(count == 80000, "4 x 20000 increments under nsync_mu = 80000");
  QT_CHECK(once_runs == 1, "nsync_run_once from 4 threads runs once");
  QT_CHECK(nsync_mu_trylock(&mu) && (nsync_mu_unlock(&mu), 1), "nsync_mu_trylock on a free lock");

  for (i = 0; i < 2; i++) pthread_create(&t[i], NULL, reader, NULL);
  for (i = 0; i < 2; i++) pthread_join(t[i], NULL);
  QT_CHECK(max_readers == 2, "two readers hold nsync_mu_rlock at the same time");

  pthread_create(&t[0], NULL, producer, NULL);
  for (i = 0; i < 100; i++) {
    nsync_mu_lock(&mu);
    while (qn == 0) nsync_cv_wait(&cv, &mu);
    sum += queue[--qn]; nsync_cv_broadcast(&cv);
    nsync_mu_unlock(&mu);
  }
  pthread_join(t[0], NULL);
  QT_CHECK(sum == 5050, "producer/consumer on nsync_cv: 1..100 sums to 5050");

  count = 0;
  pthread_create(&t[0], NULL, count_only, NULL);
  nsync_mu_lock(&mu); nsync_mu_wait(&mu, ge50, &count, NULL);
  QT_CHECK(count >= 50, "nsync_mu_wait returns once its condition (count >= 50) holds");
  nsync_mu_unlock(&mu);
  pthread_join(t[0], NULL);

  nsync_mu_lock(&mu);
  t0 = nsync_time_now();
  i = nsync_cv_wait_with_deadline(&cv, &mu, nsync_time_add(t0, nsync_time_ms(100)), NULL);
  el = nsync_time_sub(nsync_time_now(), t0);
  nsync_mu_unlock(&mu);
  QT_CHECK(i != 0 && nsync_time_cmp(el, nsync_time_ms(90)) >= 0 && nsync_time_cmp(el, nsync_time_ms(5000)) < 0,
           "nsync_cv_wait_with_deadline times out after ~100 ms");
  { nsync_counter c1 = nsync_counter_new(1);
    QT_CHECK(nsync_counter_wait(c1, nsync_time_add(nsync_time_now(), nsync_time_ms(100))) == 1, "nsync_counter_wait times out while the count is 1");
    nsync_counter_free(c1); }
  {
    nsync_note n = nsync_note_new(NULL, nsync_time_no_deadline);
    pthread_create(&t[0], NULL, notifier, n);
    QT_CHECK(nsync_note_wait(n, nsync_time_add(nsync_time_now(), nsync_time_ms(10000))) && nsync_note_is_notified(n),
             "nsync_note_wait returns when another thread notifies");
    pthread_join(t[0], NULL); nsync_note_free(n);
  }
  nsync_counter_free(done);
  QT_END();
}
