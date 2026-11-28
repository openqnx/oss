/* oss-network-info: what the Network panel plugin sees, on the command line.
 *   oss-network-info [--dump]    interfaces, addresses, default route, status
 *   oss-network-info --events N  count routing-socket messages for N seconds
 *                                (the plugin refreshes on each) */
#include "netinfo.h"
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
  if (argc == 3 && !strcmp(argv[1], "--events")) {
    int fd = net_events_open(), secs = atoi(argv[2]), n = 0;
    if (fd < 0) { perror("routing socket"); return 1; }
    printf("listening %d s\n", secs); fflush(stdout);
    time_t end = time(NULL) + secs;
    while (time(NULL) < end) {
      struct pollfd p = { fd, POLLIN, 0 };
      if (poll(&p, 1, 200) > 0) { n++; net_events_drain(fd); printf("event %d\n", n); fflush(stdout); }
    }
    printf("events %d\n", n);
    return 0;
  }
  if (argc > 1 && strcmp(argv[1], "--dump")) {
    fprintf(stderr, "usage: oss-network-info [--dump | --events SECONDS]\n");
    return 2;
  }
  NetState *s = net_read();
  if (!s->sock_ok) printf("stack none\n");
  for (guint i = 0; i < s->ifs->len; i++) {
    NetIf *f = g_ptr_array_index(s->ifs, i);
    printf("interface %s up=%d running=%d link=%s%s%s%s\n", f->name, f->up, f->running,
           f->link == 1 ? "up" : f->link == 0 ? "down" : "unknown", f->wifi ? " wifi" : "",
           f->mac[0] ? " mac=" : "", f->mac);
    for (guint k = 0; k < f->v4->len; k++) printf("  inet %s\n", (char *)g_ptr_array_index(f->v4, k));
    for (guint k = 0; k < f->v6->len; k++) printf("  inet6 %s\n", (char *)g_ptr_array_index(f->v6, k));
  }
  if (s->gw[0]) printf("default %s dev %s\n", s->gw, s->gwif);
  else printf("default none\n");
  const NetIf *p = NULL;
  NetStatus st = net_status(s, &p);
  printf("status %s%s%s\n", net_status_text(st), p ? " primary=" : "", p ? p->name : "");
  net_free(s);
  return 0;
}
