/* xtrans (the X transport layer, shipped as C source that each X library
 * compiles into itself): build it the way libICE does (ICE_t, client and
 * server, TCP and Unix-domain transports), then use it as libICE would --
 * parse transport addresses, open a TCP listener on loopback, connect a
 * client through Xtrans, accept, exchange data both ways, and check the
 * peer addresses it reports. */
#define ICE_t 1
#define TRANS_CLIENT 1
#define TRANS_SERVER 1
#define TCPCONN 1
#define UNIXCONN 1
#define HAVE_STRCASECMP 1
#define HAVE_STRLCPY 1
#include <stdio.h>
#include <string.h>
#include <X11/Xtrans/transport.c>
#include "common.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int main(void) {
  QT_BEGIN("xtrans");
  char *proto = NULL, *host = NULL, *port = NULL;
  QT_CHECK(TRANS(ParseAddress)("tcp/example.org:7000", &proto, &host, &port) && !strcmp(proto, "tcp") && !strcmp(host, "example.org") && !strcmp(port, "7000"),
           "ParseAddress tcp/example.org:7000");
  free(proto); free(host); free(port);
  QT_CHECK(TRANS(ParseAddress)("local/qnx:5", &proto, &host, &port) && !strcmp(proto, "local") && !strcmp(port, "5"), "ParseAddress local/qnx:5");
  free(proto); free(host); free(port);

  XtransConnInfo srv = TRANS(OpenCOTSServer)("tcp/127.0.0.1:0");
  QT_CHECK(srv != NULL, "OpenCOTSServer tcp");
  if (!srv) QT_END();
  int rc = TRANS(CreateListener)(srv, NULL, 0);
  QT_CHECK(rc == 0, "CreateListener (ephemeral port)");
  struct sockaddr_in sin; socklen_t sl = sizeof sin;
  getsockname(TRANS(GetConnectionNumber)(srv), (struct sockaddr *)&sin, &sl);
  int p = ntohs(sin.sin_port);
  printf("  info listening on port %d (fd %d)\n", p, TRANS(GetConnectionNumber)(srv));
  QT_CHECK(p > 0, "listener has a port");
  char addr[64]; snprintf(addr, sizeof addr, "tcp/127.0.0.1:%d", p);
  XtransConnInfo cli = TRANS(OpenCOTSClient)(addr);
  QT_CHECK(cli != NULL, "OpenCOTSClient");
  int c = cli ? TRANS(Connect)(cli, addr) : -1;
  QT_CHECK(c == 0, "Connect to the listener");
  int st = 0;
  XtransConnInfo acc = TRANS(Accept)(srv, &st);
  QT_CHECK(acc != NULL && st == 0, "Accept");
  if (acc && c == 0) {
    char msg[] = "ICE hello over xtrans", buf[64] = "";
    QT_CHECK(TRANS(Write)(cli, msg, (int)sizeof msg) == (int)sizeof msg, "client Write");
    int got = 0;
    for (int i = 0; i < 100 && got < (int)sizeof msg; i++) {
      int n = TRANS(Read)(acc, buf + got, (int)sizeof buf - got);
      if (n > 0) got += n; else usleep(10000);
    }
    QT_CHECK(got == (int)sizeof msg && !strcmp(buf, msg), "server Read the same bytes");
    char rep[] = "ack";
    TRANS(Write)(acc, rep, 3);
    BytesReadable_t pend = 0;
    for (int i = 0; i < 100 && pend < 3; i++) { TRANS(BytesReadable)(cli, &pend); if (pend < 3) usleep(10000); }
    char rb[8] = "";
    QT_CHECK(pend == 3 && TRANS(Read)(cli, rb, 3) == 3 && !strcmp(rb, "ack"), "BytesReadable 3, client reads the reply");
    int fam = 0, alen = 0; Xtransaddr *pa = NULL;
    QT_CHECK(TRANS(GetPeerAddr)(acc, &fam, &alen, &pa) == 0 && fam == AF_INET && pa
             && ((struct sockaddr_in *)pa)->sin_addr.s_addr == htonl(INADDR_LOOPBACK), "peer address is 127.0.0.1 (AF_INET)");
    free(pa);
    QT_CHECK(TRANS(IsLocal)(acc) == 0, "a TCP connection is not IsLocal");
    TRANS(Close)(acc);
  }
  if (cli) TRANS(Close)(cli);
  TRANS(Close)(srv);
  QT_END();
}
