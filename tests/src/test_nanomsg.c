/* nanomsg: scalability-protocol sockets on loopback transports.
 * PAIR over ipc:// (a socket file in /tmp) and tcp://127.0.0.1 with messages
 * both ways, REQ/REP with a computed reply, PUB/SUB over tcp where only the
 * subscribed topic arrives, PUSH/PULL over inproc:// with ordering kept, a
 * zero-copy nn_allocmsg message, nn_poll readiness, a receive timeout giving
 * ETIMEDOUT, an address without a port giving EINVAL, and nn_strerror. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/pipeline.h>

static void timeouts(int s, int ms) {
  nn_setsockopt(s, NN_SOL_SOCKET, NN_RCVTIMEO, &ms, sizeof ms);
  nn_setsockopt(s, NN_SOL_SOCKET, NN_SNDTIMEO, &ms, sizeof ms);
}
static int send_str(int s, const char *m) { return nn_send(s, m, strlen(m), 0); }
/* Receive into buf, NUL-terminated; returns the length or -1. */
static int recv_str(int s, char *buf, int cap) {
  int n = nn_recv(s, buf, cap - 1, 0);
  if (n >= 0) buf[n] = 0;
  return n;
}

/* PAIR on one address: a message each way. */
static int pair_on(const char *addr) {
  int a = nn_socket(AF_SP, NN_PAIR), b = nn_socket(AF_SP, NN_PAIR), ok = 0;
  char buf[64];
  timeouts(a, 5000); timeouts(b, 5000);
  if (nn_bind(a, addr) >= 0 && nn_connect(b, addr) >= 0) {
    ok = send_str(b, "ping") == 4 && recv_str(a, buf, sizeof buf) == 4 && strcmp(buf, "ping") == 0
      && send_str(a, "pong!") == 5 && recv_str(b, buf, sizeof buf) == 5 && strcmp(buf, "pong!") == 0;
  } else printf("  info %s: %s\n", addr, nn_strerror(nn_errno()));
  nn_close(b); nn_close(a);
  return ok;
}

int main(void) {
  QT_BEGIN("nanomsg");
  char ipc[96], tcp[64], buf[256];
  snprintf(ipc, sizeof ipc, "ipc:///tmp/qnxport-nanomsg-%d.ipc", (int)getpid());
  int port = 20000 + getpid() % 20000;

  QT_CHECK(pair_on(ipc), "PAIR over ipc:// (socket file in /tmp)");
  snprintf(tcp, sizeof tcp, "tcp://127.0.0.1:%d", port);
  QT_CHECK(pair_on(tcp), "PAIR over tcp://127.0.0.1");

  /* REQ/REP: the server answers with the request doubled. */
  {
    int rep = nn_socket(AF_SP, NN_REP), req = nn_socket(AF_SP, NN_REQ);
    timeouts(rep, 5000); timeouts(req, 5000);
    snprintf(tcp, sizeof tcp, "tcp://127.0.0.1:%d", port + 1);
    int ok = nn_bind(rep, tcp) >= 0 && nn_connect(req, tcp) >= 0;
    int good = 0;
    for (int i = 1; ok && i <= 3; i++) {
      char q[16]; snprintf(q, sizeof q, "%d", i * 21);
      if (send_str(req, q) < 0 || recv_str(rep, buf, sizeof buf) < 0) break;
      char r[32]; snprintf(r, sizeof r, "%d", 2 * atoi(buf));
      if (send_str(rep, r) < 0 || recv_str(req, buf, sizeof buf) < 0) break;
      good += atoi(buf) == i * 42;
    }
    QT_CHECK(good == 3, "REQ/REP over tcp: three requests, replies 42, 84, 126");
    nn_close(req); nn_close(rep);
  }

  /* PUB/SUB: subscribed to "qnx." only. */
  {
    int pub = nn_socket(AF_SP, NN_PUB), sub = nn_socket(AF_SP, NN_SUB);
    timeouts(sub, 300);
    snprintf(tcp, sizeof tcp, "tcp://127.0.0.1:%d", port + 2);
    nn_setsockopt(sub, NN_SUB, NN_SUB_SUBSCRIBE, "qnx.", 4);
    int ok = nn_bind(pub, tcp) >= 0 && nn_connect(sub, tcp) >= 0;
    int got_qnx = 0, got_other = 0;
    /* the connection is asynchronous: publish until the first message lands */
    for (int i = 0; ok && i < 50 && !got_qnx; i++) {
      send_str(pub, "other.ignored");
      send_str(pub, "qnx.hello");
      int n;
      while ((n = recv_str(sub, buf, sizeof buf)) >= 0) {
        if (strncmp(buf, "qnx.", 4) == 0) got_qnx++; else got_other++;
      }
    }
    QT_CHECK(got_qnx > 0, "PUB/SUB over tcp: the subscribed topic arrives");
    QT_CHECK(got_other == 0, "PUB/SUB: other topics are filtered out");
    nn_close(sub); nn_close(pub);
  }

  /* PUSH/PULL over inproc, order preserved, and a zero-copy message. */
  {
    int push = nn_socket(AF_SP, NN_PUSH), pull = nn_socket(AF_SP, NN_PULL);
    timeouts(push, 5000); timeouts(pull, 5000);
    int ok = nn_bind(pull, "inproc://qnxport") >= 0 && nn_connect(push, "inproc://qnxport") >= 0;
    int inorder = 1;
    for (int i = 0; ok && i < 100; i++) { char m[16]; snprintf(m, sizeof m, "m%03d", i); if (send_str(push, m) != 4) inorder = 0; }
    for (int i = 0; ok && i < 100; i++) {
      char m[16]; snprintf(m, sizeof m, "m%03d", i);
      if (recv_str(pull, buf, sizeof buf) != 4 || strcmp(buf, m) != 0) { inorder = 0; break; }
    }
    QT_CHECK(ok && inorder, "PUSH/PULL over inproc: 100 messages, in order");

    void *zc = nn_allocmsg(4096, 0);
    for (int i = 0; zc && i < 4096; i++) ((unsigned char *)zc)[i] = (unsigned char)(i * 7);
    QT_CHECK(zc && nn_send(push, &zc, NN_MSG, 0) == 4096, "zero-copy send of an nn_allocmsg buffer");
    struct nn_pollfd pfd = { pull, NN_POLLIN, 0 };
    QT_CHECK(nn_poll(&pfd, 1, 2000) == 1 && (pfd.revents & NN_POLLIN), "nn_poll reports the PULL socket readable");
    void *in = NULL; int n = nn_recv(pull, &in, NN_MSG, 0); int same = n == 4096;
    for (int i = 0; same && i < 4096; i++) same = ((unsigned char *)in)[i] == (unsigned char)(i * 7);
    QT_CHECK(same, "zero-copy receive: 4096 bytes intact");
    if (in) nn_freemsg(in);

    int t = 200; nn_setsockopt(pull, NN_SOL_SOCKET, NN_RCVTIMEO, &t, sizeof t);
    QT_CHECK(nn_recv(pull, buf, sizeof buf, 0) < 0 && nn_errno() == ETIMEDOUT, "receive timeout: ETIMEDOUT");
    nn_close(push); nn_close(pull);
  }

  int s = nn_socket(AF_SP, NN_PAIR);
  QT_CHECK(nn_bind(s, "tcp://127.0.0.1") < 0 && nn_errno() == EINVAL, "tcp address without a port: EINVAL");
  QT_CHECK(nn_bind(s, "bogus://x") < 0 && nn_errno() == EPROTONOSUPPORT, "unknown transport: EPROTONOSUPPORT");
  nn_close(s);
  QT_CHECK(strlen(nn_strerror(ETIMEDOUT)) > 0, "nn_strerror");
  nn_term();
  unlink(ipc + 6);
  QT_END();
}
