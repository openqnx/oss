/* libevent: an event_base with a socketpair read event and a timeout. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <event2/event.h>
#include <event2/buffer.h>
static int read_fired, timer_fired, got;
static void on_read(evutil_socket_t fd, short what, void *arg) {
  char buf[16];
  ssize_t n = recv(fd, buf, sizeof buf, 0);
  if (n > 0) got += (int)n;
  ++read_fired;
}
static void on_timeout(evutil_socket_t fd, short what, void *arg) {
  ++timer_fired;
  event_base_loopbreak((struct event_base *)arg);
}
int main(void) {
  QT_BEGIN("libevent");
  printf("  info libevent %s\n", event_get_version());
  struct event_base *base = event_base_new();
  QT_CHECK(base != NULL, "event_base_new");
  const char *m = event_base_get_method(base);
  printf("  info backend = %s\n", m ? m : "(none)");
  QT_CHECK(m != NULL, "a backend method was selected");
  int sv[2];
  QT_CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0, "socketpair");
  evutil_make_socket_nonblocking(sv[0]);
  QT_CHECK(send(sv[1], "qnx", 3, 0) == 3, "send 3 bytes");
  struct event *rev = event_new(base, sv[0], EV_READ, on_read, NULL);
  event_add(rev, NULL);
  struct timeval tv = { 0, 100000 };
  struct event *tev = event_new(base, -1, 0, on_timeout, base);
  event_add(tev, &tv);
  event_base_dispatch(base);
  QT_CHECK(read_fired == 1, "read event fired");
  QT_CHECK(got == 3, "read event delivered 3 bytes");
  QT_CHECK(timer_fired == 1, "timeout fired");

  struct evbuffer *buf = evbuffer_new();
  evbuffer_add_printf(buf, "sdp=%d", 8);
  QT_CHECK(evbuffer_get_length(buf) == 5, "evbuffer_add_printf");
  char out[8] = {0};
  evbuffer_remove(buf, out, 5);
  QT_CHECK(strcmp(out, "sdp=8") == 0, "evbuffer_remove");
  evbuffer_free(buf);
  event_free(rev); event_free(tev);
  close(sv[0]); close(sv[1]);
  event_base_free(base);
  QT_END();
}
