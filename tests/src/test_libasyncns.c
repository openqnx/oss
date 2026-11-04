/* libasyncns: name lookups answered by its worker threads while the caller
 * waits on asyncns_fd(). Checks: getaddrinfo("127.0.0.1", "80") ->
 * AF_INET 127.0.0.1 port 80; getaddrinfo("localhost") -> a loopback
 * address; getnameinfo of 127.0.0.1:22 numeric -> "127.0.0.1", "22";
 * three queries outstanding at once all complete; a cancelled query
 * leaves the others intact; a name in the reserved .invalid domain fails
 * with an EAI_ error. */
#include "common.h"
#include <stdlib.h>
#include <poll.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <asyncns.h>

static asyncns_t *as;
static int wait_done(asyncns_query_t *q) {
  for (int i = 0; i < 1200 && !asyncns_isdone(as, q); i++) {
    struct pollfd p = { asyncns_fd(as), POLLIN, 0 };
    poll(&p, 1, 50);
    if (asyncns_wait(as, 0) < 0) return 0;
  }
  return asyncns_isdone(as, q);
}
int main(void) {
  QT_BEGIN("libasyncns");
  as = asyncns_new(2);
  QT_CHECK(as != NULL, "asyncns_new(2 workers)");
  if (!as) QT_END();
  struct addrinfo hints; memset(&hints, 0, sizeof hints); hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICHOST; hints.ai_family = AF_INET;
  asyncns_query_t *q = asyncns_getaddrinfo(as, "127.0.0.1", "80", &hints);
  struct addrinfo *res = NULL;
  int ok = q && wait_done(q) && asyncns_getaddrinfo_done(as, q, &res) == 0 && res && res->ai_family == AF_INET;
  struct sockaddr_in *sin = ok ? (struct sockaddr_in *)res->ai_addr : NULL;
  QT_CHECK(ok && sin->sin_addr.s_addr == htonl(INADDR_LOOPBACK) && ntohs(sin->sin_port) == 80, "getaddrinfo(127.0.0.1, 80): AF_INET 127.0.0.1:80");
  if (res) asyncns_freeaddrinfo(res);

  memset(&hints, 0, sizeof hints); hints.ai_socktype = SOCK_STREAM;
  q = asyncns_getaddrinfo(as, "localhost", NULL, &hints); res = NULL;
  int r = (q && wait_done(q)) ? asyncns_getaddrinfo_done(as, q, &res) : -1;
  int loop = 0;
  for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
    if (ai->ai_family == AF_INET && ((struct sockaddr_in *)ai->ai_addr)->sin_addr.s_addr == htonl(INADDR_LOOPBACK)) loop = 1;
    if (ai->ai_family == AF_INET6 && IN6_IS_ADDR_LOOPBACK(&((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr)) loop = 1;
  }
  QT_CHECK(r == 0 && loop, "getaddrinfo(localhost): a loopback address (from /etc/hosts)");
  if (res) asyncns_freeaddrinfo(res);

  struct sockaddr_in sa; memset(&sa, 0, sizeof sa); sa.sin_family = AF_INET; sa.sin_port = htons(22); sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  q = asyncns_getnameinfo(as, (struct sockaddr *)&sa, sizeof sa, NI_NUMERICHOST | NI_NUMERICSERV, 1, 1);
  char host[64] = "", serv[16] = "";
  r = (q && wait_done(q)) ? asyncns_getnameinfo_done(as, q, host, sizeof host, serv, sizeof serv) : -1;
  QT_CHECK(r == 0 && !strcmp(host, "127.0.0.1") && !strcmp(serv, "22"), "getnameinfo(127.0.0.1:22, numeric) -> \"127.0.0.1\", \"22\"");

  memset(&hints, 0, sizeof hints); hints.ai_flags = AI_NUMERICHOST; hints.ai_family = AF_INET;
  const char *names[3] = { "10.1.2.3", "192.168.7.8", "172.16.0.9" };
  asyncns_query_t *qs[3]; for (int i = 0; i < 3; i++) qs[i] = asyncns_getaddrinfo(as, names[i], NULL, &hints);
  asyncns_query_t *cq = asyncns_getaddrinfo(as, "10.9.9.9", NULL, &hints);
  asyncns_cancel(as, cq);
  int good = 0;
  for (int i = 0; i < 3; i++) {
    res = NULL;
    if (qs[i] && wait_done(qs[i]) && asyncns_getaddrinfo_done(as, qs[i], &res) == 0 && res) {
      char b[32]; inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, b, sizeof b);
      if (!strcmp(b, names[i])) good++;
    }
    if (res) asyncns_freeaddrinfo(res);
  }
  QT_CHECK(good == 3, "three outstanding queries complete, each with its own answer (a fourth cancelled)");
  QT_CHECK(asyncns_getnqueries(as) == 0, "no queries left outstanding");

  memset(&hints, 0, sizeof hints); hints.ai_socktype = SOCK_STREAM;
  q = asyncns_getaddrinfo(as, "no-such-host.invalid", NULL, &hints); res = NULL;
  r = (q && wait_done(q)) ? asyncns_getaddrinfo_done(as, q, &res) : 0;
  printf("  info no-such-host.invalid: %d (%s)\n", r, gai_strerror(r));
  QT_CHECK(r != 0 && res == NULL, "a .invalid name fails with an EAI_ error");
  asyncns_free(as);
  QT_END();
}
