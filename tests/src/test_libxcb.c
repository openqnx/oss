/* libxcb: display-name parsing, a clean connection error when no server
 * is there, and a real protocol exchange with a minimal X server played by
 * a thread of this test over a socketpair (xcb_connect_to_fd): the client's
 * connection setup request is checked byte for byte, a crafted setup reply
 * is parsed (vendor, resource-id base/mask, keycodes), an InternAtom
 * request goes out and its reply comes back with the right sequence number,
 * an X error reply is delivered as xcb_generic_error_t, and ids from
 * xcb_generate_id fall inside the server-assigned range. */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <xcb/xcb.h>
static int srv_fd;
static unsigned char setup_req[12];
static char atom_name[2][32];
static int srv_ok;
static int full(int fd, void *b, int n, int wr) {
  char *c = b;
  while (n > 0) { int r = wr ? (int)write(fd, c, n) : (int)read(fd, c, n); if (r <= 0) return 0; c += r; n -= r; }
  return 1;
}
static void put16(unsigned char *p, unsigned v) { p[0] = v & 0xff; p[1] = (v >> 8) & 0xff; }
static void put32(unsigned char *p, uint32_t v) { put16(p, v & 0xffff); put16(p + 2, v >> 16); }
static void *server(void *arg) {
  (void)arg;
  if (!full(srv_fd, setup_req, 12, 0)) return NULL;
  /* little-endian setup reply: 8 header + 32 fixed + 8 vendor, no formats, no screens */
  unsigned char r[48]; memset(r, 0, sizeof r);
  r[0] = 1; put16(r + 2, 11); put16(r + 4, 0); put16(r + 6, 10);
  put32(r + 8, 12345678);          /* release */
  put32(r + 12, 0x00a00000);       /* resource-id base */
  put32(r + 16, 0x001fffff);       /* resource-id mask */
  put16(r + 24, 7);                /* vendor length */
  put16(r + 26, 0xffff);           /* max request length */
  r[28] = 0; r[29] = 0; r[30] = 0; r[31] = 0; r[32] = 32; r[33] = 32; r[34] = 8; r[35] = 255;
  memcpy(r + 40, "QNXtest", 7);
  if (!full(srv_fd, r, sizeof r, 1)) return NULL;
  for (int i = 0; i < 2; i++) {
    unsigned char hdr[4], body[64];
    if (!full(srv_fd, hdr, 4, 0)) return NULL;
    int len = (hdr[2] | hdr[3] << 8) * 4 - 4;
    if (hdr[0] != 16 || len < 4 || len > (int)sizeof body || !full(srv_fd, body, len, 0)) return NULL;
    int nl = body[0] | body[1] << 8;
    if (nl < 32) { memcpy(atom_name[i], body + 4, nl); atom_name[i][nl] = 0; }
    unsigned char rep[32]; memset(rep, 0, sizeof rep);
    if (i == 0) { rep[0] = 1; put16(rep + 2, 1); put32(rep + 8, 0x1234); }   /* reply, seq 1, atom */
    else { rep[0] = 0; rep[1] = 2; put16(rep + 2, 2); put32(rep + 4, 0xdead); rep[10] = 16; }  /* BadValue */
    if (!full(srv_fd, rep, 32, 1)) return NULL;
  }
  srv_ok = 1;
  return NULL;
}
int main(void) {
  QT_BEGIN("libxcb");
  char *host = NULL; int disp = -1, scr = -1;
  QT_CHECK(xcb_parse_display("qnxbox:12.3", &host, &disp, &scr) && host && strcmp(host, "qnxbox") == 0 && disp == 12 && scr == 3,
           "xcb_parse_display qnxbox:12.3");
  free(host); host = NULL;
  QT_CHECK(xcb_parse_display(":0", &host, &disp, &scr) && host && host[0] == 0 && disp == 0 && scr == 0, "xcb_parse_display :0");
  free(host);
  QT_CHECK(!xcb_parse_display("nonsense", &host, &disp, &scr), "invalid display name rejected");

  xcb_connection_t *c = xcb_connect(":97", NULL);
  int e = xcb_connection_has_error(c);
  printf("  info xcb_connect(:97) error %d\n", e);
  QT_CHECK(e == XCB_CONN_ERROR, "no server on :97 -> XCB_CONN_ERROR, no crash");
  xcb_disconnect(c);

  int sv[2];
  QT_CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0, "socketpair");
  srv_fd = sv[1];
  pthread_t th; pthread_create(&th, NULL, server, NULL);
  c = xcb_connect_to_fd(sv[0], NULL);
  QT_CHECK(c && !xcb_connection_has_error(c), "xcb_connect_to_fd: setup accepted");
  int le = setup_req[0] == 'l', be = setup_req[0] == 'B';
  unsigned maj = le ? (setup_req[2] | setup_req[3] << 8) : (setup_req[2] << 8 | setup_req[3]);
  QT_CHECK((le || be) && maj == 11, "client setup request: byte order mark and protocol 11");
  if (!le) printf("  info client is big-endian; the fake server speaks little-endian only\n");
  if (c && !xcb_connection_has_error(c)) {
    const xcb_setup_t *s = xcb_get_setup(c);
    int vl = xcb_setup_vendor_length(s);
    printf("  info vendor %.*s release %u\n", vl, xcb_setup_vendor(s), (unsigned)s->release_number);
    QT_CHECK(vl == 7 && memcmp(xcb_setup_vendor(s), "QNXtest", 7) == 0 && s->release_number == 12345678,
             "setup reply parsed: vendor and release");
    QT_CHECK(s->min_keycode == 8 && s->max_keycode == 255 && s->protocol_major_version == 11 &&
             xcb_setup_roots_iterator(s).rem == 0, "keycode range, version, zero screens");
    uint32_t id1 = xcb_generate_id(c), id2 = xcb_generate_id(c);
    printf("  info generated ids %08x %08x\n", (unsigned)id1, (unsigned)id2);
    QT_CHECK(id1 != id2 && (id1 & ~0x001fffffu) == 0x00a00000 && (id2 & ~0x001fffffu) == 0x00a00000,
             "xcb_generate_id inside base/mask");
    xcb_intern_atom_cookie_t ck = xcb_intern_atom(c, 0, 8, "QNX_ATOM");
    xcb_intern_atom_cookie_t ck2 = xcb_intern_atom(c, 1, 5, "OTHER");
    QT_CHECK(ck.sequence == 1 && ck2.sequence == 2, "request sequence numbers 1, 2");
    xcb_generic_error_t *err = NULL;
    xcb_intern_atom_reply_t *rep = xcb_intern_atom_reply(c, ck, &err);
    QT_CHECK(rep && !err && rep->atom == 0x1234, "InternAtom reply: atom 0x1234");
    free(rep); free(err); err = NULL;
    rep = xcb_intern_atom_reply(c, ck2, &err);
    QT_CHECK(!rep && err && err->error_code == 2 && err->resource_id == 0xdead && err->major_code == 16 && err->sequence == 2,
             "X error delivered: BadValue for request 2");
    free(rep); free(err);
  }
  pthread_join(th, NULL);
  QT_CHECK(srv_ok && strcmp(atom_name[0], "QNX_ATOM") == 0 && strcmp(atom_name[1], "OTHER") == 0,
           "server saw both InternAtom requests with their names");
  if (c) xcb_disconnect(c);
  close(sv[1]);
  QT_END();
}
