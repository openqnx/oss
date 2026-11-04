/* datefudge: the program test_datefudge runs under datefudge. With an
 * argument it prints time(), gettimeofday() and clock_gettime(
 * CLOCK_REALTIME) seconds and whether CLOCK_MONOTONIC still advances; run
 * alone it checks that the three real-time clocks agree. */
#include "common.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char **argv) {
  struct timeval tv; struct timespec rt, m1, m2;
  time_t t = time(NULL);
  gettimeofday(&tv, NULL);
  clock_gettime(CLOCK_REALTIME, &rt);
  clock_gettime(CLOCK_MONOTONIC, &m1);
  if (argc > 1) {
    usleep(20000);
    clock_gettime(CLOCK_MONOTONIC, &m2);
    printf("%lld %lld %lld %s\n", (long long)t, (long long)tv.tv_sec, (long long)rt.tv_sec,
           (m2.tv_sec > m1.tv_sec || (m2.tv_sec == m1.tv_sec && m2.tv_nsec > m1.tv_nsec)) ? "mono-ok" : "mono-stuck");
    return 0;
  }
  QT_BEGIN("datefudge_clock");
  QT_CHECK(llabs((long long)tv.tv_sec - t) <= 1 && llabs((long long)rt.tv_sec - t) <= 1, "time, gettimeofday and clock_gettime agree");
  QT_CHECK(t > 1700000000, "the clock is after 2023");
  QT_END();
}
