/* libXdmcp: build an XDMCP Query packet (header + ARRAYofARRAY8 of auth
 * names) and a Request-style body of CARD8/16/32 and ARRAY16, send it with
 * XdmcpFlush over a loopback UDP socket, receive it with XdmcpFill, parse
 * it back and compare -- including the exact big-endian wire bytes; plus
 * ARRAY8 helpers and the XDM-AUTHORIZATION-1 DES wrap/unwrap round trip. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <X11/Xdmcp.h>
/* exported by the library; the header declares them only with HASXDMAUTH */
extern void XdmcpWrap(unsigned char *, unsigned char *, unsigned char *, int);
extern void XdmcpUnwrap(unsigned char *, unsigned char *, unsigned char *, int);
static void set8(ARRAY8 *a, const char *s) { XdmcpAllocARRAY8(a, (int)strlen(s)); memcpy(a->data, s, strlen(s)); }
int main(void) {
  QT_BEGIN("libxdmcp");
  ARRAY8 x, y;
  set8(&x, "MIT-MAGIC-COOKIE-1");
  QT_CHECK(XdmcpCopyARRAY8(&x, &y) && XdmcpARRAY8Equal(&x, &y), "ARRAY8 copy compares equal");
  y.data[0] = 'm';
  QT_CHECK(!XdmcpARRAY8Equal(&x, &y), "modified copy differs");
  QT_CHECK(XdmcpReallocARRAY8(&y, 40) && y.length == 40 && y.data[1] == 'I', "XdmcpReallocARRAY8 keeps contents");
  XdmcpDisposeARRAY8(&y);

  /* packet */
  ARRAYofARRAY8 names;
  XdmcpAllocARRAYofARRAY8(&names, 2);
  set8(&names.data[0], "MIT-MAGIC-COOKIE-1");
  set8(&names.data[1], "XDM-AUTHORIZATION-1");
  ARRAY16 conn; XdmcpAllocARRAY16(&conn, 2); conn.data[0] = 0; conn.data[1] = 6;
  XdmcpBuffer out = { (BYTE *)malloc(XDM_MAX_MSGLEN), XDM_MAX_MSGLEN, 0, 0 };
  int body = 1 + (2 + 18) + (2 + 19) + 1 + 2 * 2 + 2 + 4 + 1;
  XdmcpHeader h = { XDM_PROTOCOL_VERSION, QUERY, (CARD16)body };
  int ok = XdmcpWriteHeader(&out, &h) && XdmcpWriteARRAYofARRAY8(&out, &names) && XdmcpWriteARRAY16(&out, &conn) &&
           XdmcpWriteCARD16(&out, 0xBEEF) && XdmcpWriteCARD32(&out, 0x01020304) && XdmcpWriteCARD8(&out, 0x7f);
  QT_CHECK(ok && out.pointer == 6 + body, "packet marshalled, length as computed");
  const BYTE head[] = { 0, 1, 0, 2, 0, (BYTE)body, 2, 0, 18, 'M', 'I', 'T' };
  QT_CHECK(memcmp(out.data, head, sizeof head) == 0, "header and first ARRAY8 are big-endian on the wire");

  int rs = socket(AF_INET, SOCK_DGRAM, 0), ss = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in a; memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  socklen_t al = sizeof a;
  ok = rs >= 0 && ss >= 0 && bind(rs, (struct sockaddr *)&a, sizeof a) == 0 && getsockname(rs, (struct sockaddr *)&a, &al) == 0;
  QT_CHECK(ok, "UDP socket bound on 127.0.0.1");
  printf("  info XDMCP peer on 127.0.0.1:%d\n", ntohs(a.sin_port));
  QT_CHECK(XdmcpFlush(ss, &out, (XdmcpNetaddr)&a, sizeof a), "XdmcpFlush sends the datagram");
  XdmcpBuffer in = { (BYTE *)malloc(XDM_MAX_MSGLEN), XDM_MAX_MSGLEN, 0, 0 };
  struct sockaddr_in from; int fl = sizeof from;
  QT_CHECK(XdmcpFill(rs, &in, (XdmcpNetaddr)&from, &fl) && in.count == out.pointer, "XdmcpFill receives it whole");
  XdmcpHeader rh;
  QT_CHECK(XdmcpReadHeader(&in, &rh) && rh.version == XDM_PROTOCOL_VERSION && rh.opcode == QUERY && rh.length == body,
           "XdmcpReadHeader: version 1, QUERY, length");
  ARRAYofARRAY8 rn = { 0, NULL };
  ARRAY16 rc = { 0, NULL };
  CARD16 c16 = 0; CARD32 c32 = 0; CARD8 c8 = 0;
  ok = XdmcpReadARRAYofARRAY8(&in, &rn) && XdmcpReadARRAY16(&in, &rc) && XdmcpReadCARD16(&in, &c16) &&
       XdmcpReadCARD32(&in, &c32) && XdmcpReadCARD8(&in, &c8);
  QT_CHECK(ok && rn.length == 2 && XdmcpARRAY8Equal(&rn.data[0], &names.data[0]) && XdmcpARRAY8Equal(&rn.data[1], &names.data[1]),
           "auth names read back");
  QT_CHECK(rc.length == 2 && rc.data[1] == 6 && c16 == 0xBEEF && c32 == 0x01020304 && c8 == 0x7f, "ARRAY16 and CARD8/16/32 read back");
  CARD8 extra;
  QT_CHECK(!XdmcpReadCARD8(&in, &extra), "reading past the end fails");
  XdmcpDisposeARRAYofARRAY8(&rn); XdmcpDisposeARRAY16(&rc);
  XdmcpDisposeARRAYofARRAY8(&names); XdmcpDisposeARRAY16(&conn); XdmcpDisposeARRAY8(&x);
  close(rs); close(ss); free(out.data); free(in.data);

  /* keys and DES wrapping */
  XdmAuthKeyRec k1 = { { 0, 1, 2, 3, 4, 5, 6, 0xff } }, k2 = k1;
  XdmcpIncrementKey(&k2);
  QT_CHECK(!XdmcpCompareKeys(&k1, &k2) && k2.data[7] == 0 && k2.data[6] == 7, "XdmcpIncrementKey carries");
  XdmcpDecrementKey(&k2);
  QT_CHECK(XdmcpCompareKeys(&k1, &k2), "XdmcpDecrementKey undoes it");
  unsigned char key[8] = { 0, 0x13, 0x57, 0x9b, 0xdf, 0x24, 0x68, 0xac };
  unsigned char plain[16] = "QNX xdmcp block", enc[16], dec[16];
  memcpy(plain + 8, plain, 8);   /* two equal blocks: chaining must make them differ */
  XdmcpWrap(plain, key, enc, 16);
  QT_CHECK(memcmp(enc, plain, 8) != 0 && memcmp(enc, enc + 8, 8) != 0, "XdmcpWrap encrypts with chaining");
  XdmcpUnwrap(enc, key, dec, 16);
  QT_CHECK(memcmp(dec, plain, 16) == 0, "XdmcpUnwrap restores the plaintext");
  QT_END();
}
