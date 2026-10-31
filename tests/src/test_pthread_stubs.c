/* pthread-stubs: a .pc file saying where the POSIX threads functions are
 * (in QNX's libc, so its Libs are empty). Link through it, as libxcb's
 * users do, and use the threads API: threads, mutex, condition variable,
 * once, thread-specific keys, rwlock, barrier -- with known results. */
#include "common.h"
#include <pthread.h>

static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_key_t key;
static pthread_barrier_t bar;
static pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
static long counter, ready, once_calls, rw_sum;
static int dtor_calls;

static void init_once(void) { once_calls++; }
static void dtor(void *p) { (void)p; __sync_fetch_and_add(&dtor_calls, 1); }

static void *worker(void *arg) {
  long id = (long)arg;
  pthread_once(&once, init_once);
  pthread_setspecific(key, (void *)(id + 1));
  for (int i = 0; i < 10000; i++) { pthread_mutex_lock(&mu); counter++; pthread_mutex_unlock(&mu); }
  pthread_rwlock_wrlock(&rw); rw_sum += id; pthread_rwlock_unlock(&rw);
  pthread_barrier_wait(&bar);
  pthread_mutex_lock(&mu); ready++; pthread_cond_signal(&cv); pthread_mutex_unlock(&mu);
  return (void *)((long)pthread_getspecific(key) * 10);
}

int main(void) {
  QT_BEGIN("pthread-stubs");
  QT_CHECK(pthread_key_create(&key, dtor) == 0, "pthread_key_create");
  QT_CHECK(pthread_barrier_init(&bar, NULL, 4) == 0, "pthread_barrier_init(4)");
  pthread_t t[4]; int ok = 1;
  for (long i = 0; i < 4; i++) ok &= pthread_create(&t[i], NULL, worker, (void *)i) == 0;
  QT_CHECK(ok, "4 threads created");
  pthread_mutex_lock(&mu);
  while (ready < 4) pthread_cond_wait(&cv, &mu);
  pthread_mutex_unlock(&mu);
  QT_CHECK(ready == 4, "condition variable: all 4 signalled");
  long rets = 0;
  for (int i = 0; i < 4; i++) { void *r; pthread_join(t[i], &r); rets += (long)r; }
  QT_CHECK(rets == (1 + 2 + 3 + 4) * 10, "each thread saw its own thread-specific value");
  QT_CHECK(counter == 40000, "mutex: 4 x 10000 increments = 40000");
  QT_CHECK(once_calls == 1, "pthread_once ran once");
  QT_CHECK(rw_sum == 0 + 1 + 2 + 3, "rwlock-protected sum 6");
  QT_CHECK(dtor_calls == 4, "key destructor ran for each thread");
  QT_CHECK(pthread_equal(pthread_self(), pthread_self()) && !pthread_equal(t[0], t[1]), "pthread_equal");
  pthread_barrier_destroy(&bar);
  pthread_key_delete(key);
  QT_END();
}
