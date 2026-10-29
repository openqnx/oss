/* nng: req/rep over inproc and tcp. */
#include "common.h"
#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/reqrep0/rep.h>
static int roundtrip(const char *url) {
  nng_socket rep, req; char *buf = NULL; size_t sz;
  if (nng_rep0_open(&rep) || nng_req0_open(&req)) return 0;
  if (nng_listen(rep, url, NULL, 0) || nng_dial(req, url, NULL, 0)) return 0;
  if (nng_send(req, "ping", 5, 0) || nng_recv(rep, &buf, &sz, NNG_FLAG_ALLOC)) return 0;
  int ok = sz == 5 && !strcmp(buf, "ping"); nng_free(buf, sz);
  nng_close(req); nng_close(rep); return ok;
}
int main(void) {
  QT_BEGIN("nng");
  QT_CHECK(roundtrip("inproc://t"), "inproc req/rep");
  QT_CHECK(roundtrip("tcp://127.0.0.1:23456"), "tcp req/rep");
  QT_END();
}
