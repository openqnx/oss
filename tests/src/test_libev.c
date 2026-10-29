/* libev: a default loop driving an io watcher on a pipe plus a timer. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <ev.h>
static int io_fired, timer_fired, got;
static void io_cb(EV_P_ ev_io *w, int revents) {
  char buf[16];
  ssize_t n = read(w->fd, buf, sizeof buf);
  if (n > 0) got += (int)n;
  ++io_fired;
  ev_io_stop(EV_A_ w);
}
static void timer_cb(EV_P_ ev_timer *w, int revents) {
  ++timer_fired;
  ev_break(EV_A_ EVBREAK_ALL);
}
int main(void) {
  QT_BEGIN("libev");
  printf("  info libev %d.%d\n", ev_version_major(), ev_version_minor());
  struct ev_loop *loop = ev_default_loop(0);
  QT_CHECK(loop != NULL, "ev_default_loop");
  QT_CHECK(ev_backend(loop) != 0, "a backend was selected");
  /* epoll is linked from QNX's libepoll; make sure libev both offers it and
     picked it, so the io watcher below runs through that backend. */
  printf("  info backend 0x%x (supported 0x%x)\n", ev_backend(loop), ev_supported_backends());
  QT_CHECK(ev_backend(loop) == EVBACKEND_EPOLL, "the default loop uses epoll");
  int fds[2];
  QT_CHECK(pipe(fds) == 0, "pipe");
  QT_CHECK(write(fds[1], "qnx", 3) == 3, "write to pipe");
  ev_io io;
  ev_io_init(&io, io_cb, fds[0], EV_READ);
  ev_io_start(loop, &io);
  ev_timer timer;
  ev_timer_init(&timer, timer_cb, 0.05, 0.0);
  ev_timer_start(loop, &timer);
  ev_run(loop, 0);
  QT_CHECK(io_fired == 1, "io watcher fired");
  QT_CHECK(got == 3, "io watcher read 3 bytes");
  QT_CHECK(timer_fired == 1, "timer fired and broke the loop");
  QT_CHECK(ev_time() > 0, "ev_time");
  close(fds[0]); close(fds[1]);
  QT_END();
}
