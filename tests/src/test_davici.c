/* davici: the VICI client library against a VICI server in this process
 * (a thread on an AF_UNIX socket, then on TCP loopback) that speaks the wire
 * protocol strongSwan's charon speaks: 4-byte big-endian packet length,
 * packet type, name, then the message elements. Checks, with known answers:
 *  - a "version" command: the request on the wire is byte for byte what
 *    the protocol says, and the response (daemon/version/sysname key-values)
 *    parses into the expected elements;
 *  - a nested request (section, key-value, list with two items, a 3000-byte
 *    value) echoed back by the server parses back element by element, and
 *    davici_dump() prints it as expected;
 *  - an unknown command ends with -ENOSYS;
 *  - event registration is confirmed, two events are delivered with their
 *    payloads, and unregistration is confirmed;
 *  - the same command over TCP (davici_connect_tcp) on 127.0.0.1. */
#include "common.h"
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <davici.h>

enum { CMD_REQUEST, CMD_RESPONSE, CMD_UNKNOWN, EVENT_REGISTER, EVENT_UNREGISTER,
       EVENT_CONFIRM, EVENT_UNKNOWN, EVENT };
enum { SEC_START = 1, SEC_END, KV, LIST_START, LIST_ITEM, LIST_END };

static int listen_fd = -1;
static int srv_errors = 0;
static int version_req_ok = 0;
static char srv_note[256];

/* ---- server side: blocking I/O on the accepted socket ---- */
static int readn(int fd, void *buf, size_t n) {
  size_t got = 0;
  while (got < n) {
    ssize_t r = read(fd, (char *)buf + got, n - got);
    if (r <= 0) return -1;
    got += (size_t)r;
  }
  return 0;
}
static int writen(int fd, const void *buf, size_t n) {
  size_t put = 0;
  while (put < n) {
    ssize_t r = write(fd, (const char *)buf + put, n - put);
    if (r <= 0) return -1;
    put += (size_t)r;
  }
  return 0;
}
/* One packet: type, name (for named types), body. */
static int send_packet(int fd, uint8_t type, const char *name, const void *body, size_t blen) {
  uint8_t hdr[4 + 1 + 1 + 255];
  size_t nlen = name ? strlen(name) : 0, h = 5;
  uint32_t len = (uint32_t)(1 + (name ? 1 + nlen : 0) + blen);
  len = htonl(len);
  memcpy(hdr, &len, 4);
  hdr[4] = type;
  if (name) { hdr[h++] = (uint8_t)nlen; memcpy(hdr + h, name, nlen); h += nlen; }
  if (writen(fd, hdr, h) || (blen && writen(fd, body, blen))) return -1;
  return 0;
}
/* Body builders. */
static size_t put_named(uint8_t *p, uint8_t type, const char *name) {
  size_t n = strlen(name);
  p[0] = type; p[1] = (uint8_t)n; memcpy(p + 2, name, n);
  return 2 + n;
}
static size_t put_kv(uint8_t *p, const char *name, const char *value) {
  size_t o = put_named(p, KV, name), v = strlen(value);
  p[o] = (uint8_t)(v >> 8); p[o + 1] = (uint8_t)v; memcpy(p + o + 2, value, v);
  return o + 2 + v;
}

static void serve(int fd) {
  uint8_t *buf = malloc(65536);
  int events_sent = 0;
  for (;;) {
    uint32_t len;
    if (readn(fd, &len, 4)) break;
    len = ntohl(len);
    if (len < 1 || len > 65535 || readn(fd, buf, len)) { srv_errors++; break; }
    uint8_t type = buf[0];
    char name[256] = "";
    size_t off = 1;
    if (type == CMD_REQUEST || type == EVENT_REGISTER || type == EVENT_UNREGISTER) {
      memcpy(name, buf + 2, buf[1]); name[buf[1]] = 0; off = 2 + buf[1];
    }
    if (type == CMD_REQUEST && !strcmp(name, "version")) {
      /* the whole packet as the protocol defines it: no message elements */
      version_req_ok = (len == 1 + 1 + 7 && off == len);
      uint8_t b[256]; size_t o = 0;
      o += put_kv(b + o, "daemon", "charon");
      o += put_kv(b + o, "version", "6.0.6");
      o += put_kv(b + o, "sysname", "QNX");
      send_packet(fd, CMD_RESPONSE, NULL, b, o);
    } else if (type == CMD_REQUEST && !strcmp(name, "echo")) {
      send_packet(fd, CMD_RESPONSE, NULL, buf + off, len - off);   /* body back */
    } else if (type == CMD_REQUEST) {
      send_packet(fd, CMD_UNKNOWN, NULL, NULL, 0);
    } else if (type == EVENT_REGISTER && !strcmp(name, "log")) {
      send_packet(fd, EVENT_CONFIRM, NULL, NULL, 0);
      for (int i = 0; i < 2; i++) {
        uint8_t b[128]; char msg[32];
        snprintf(msg, sizeof msg, "message %d", i + 1);
        size_t o = put_kv(b, "msg", msg);
        send_packet(fd, EVENT, "log", b, o);
        events_sent++;
      }
    } else if (type == EVENT_UNREGISTER && !strcmp(name, "log")) {
      send_packet(fd, EVENT_CONFIRM, NULL, NULL, 0);
    } else if (type == EVENT_REGISTER || type == EVENT_UNREGISTER) {
      send_packet(fd, EVENT_UNKNOWN, NULL, NULL, 0);
    } else {
      snprintf(srv_note, sizeof srv_note, "unexpected packet type %d", type);
      srv_errors++;
    }
  }
  (void)events_sent;
  free(buf);
}
static void *server_thread(void *arg) {
  int conns = (int)(intptr_t)arg;
  for (int i = 0; i < conns; i++) {
    int fd = accept(listen_fd, NULL, NULL);
    if (fd < 0) { srv_errors++; break; }
    serve(fd);
    close(fd);
  }
  return NULL;
}

/* ---- client side: davici driven by poll() ---- */
static struct pollfd cpfd = { .fd = -1 };
static int done;
static int iocb(struct davici_conn *c, int fd, int ops, void *user) {
  (void)c; (void)user;
  cpfd.fd = fd;
  cpfd.events = (short)(((ops & DAVICI_READ) ? POLLIN : 0) | ((ops & DAVICI_WRITE) ? POLLOUT : 0));
  return 0;
}
static int run_until(struct davici_conn *c, int *flag, int want) {
  for (int spins = 0; *flag < want && spins < 2000; spins++) {
    int r = poll(&cpfd, 1, 2000);
    if (r <= 0) return -1;
    if ((cpfd.revents & POLLIN) && davici_read(c) < 0) return -1;
    if ((cpfd.revents & POLLOUT) && davici_write(c) < 0) return -1;
  }
  return *flag >= want ? 0 : -1;
}

static char version_seen[128];
static void versioncb(struct davici_conn *c, int err, const char *name,
                      struct davici_response *res, void *user) {
  (void)c; (void)user;
  if (err >= 0 && res && !strcmp(name, "version")) {
    char k[3][16], v[3][16]; int n = 0, ret;
    while ((ret = davici_parse(res)) == DAVICI_KEY_VALUE && n < 3) {
      snprintf(k[n], sizeof k[n], "%s", davici_get_name(res));
      davici_get_value_str(res, v[n], sizeof v[n]);
      n++;
    }
    if (ret == DAVICI_END && n == 3)
      snprintf(version_seen, sizeof version_seen, "%s=%s %s=%s %s=%s", k[0], v[0], k[1], v[1], k[2], v[2]);
  }
  done++;
}

static char big[3000];
static int echo_ok, echo_elems;
static char dumped[8192];
static void echocb(struct davici_conn *c, int err, const char *name,
                   struct davici_response *res, void *user) {
  (void)c; (void)name; (void)user;
  static const int want[] = { DAVICI_SECTION_START, DAVICI_KEY_VALUE, DAVICI_LIST_START,
                              DAVICI_LIST_ITEM, DAVICI_LIST_ITEM, DAVICI_LIST_END,
                              DAVICI_KEY_VALUE, DAVICI_SECTION_END, DAVICI_END };
  int ok = err >= 0 && res != NULL;
  for (int i = 0; ok && i < (int)(sizeof want / sizeof want[0]); i++) {
    int ret = davici_parse(res);
    echo_elems++;
    if (ret != want[i]) { ok = 0; break; }
    unsigned int len; const void *v;
    switch (i) {
      case 0: ok = !strcmp(davici_get_name(res), "conn") && davici_get_level(res) == 1; break;
      case 1: ok = !strcmp(davici_get_name(res), "remote_addrs") && davici_value_strcmp(res, "127.0.0.2") == 0; break;
      case 2: ok = !strcmp(davici_get_name(res), "proposals") && davici_get_level(res) == 2; break;
      case 3: ok = davici_value_strcmp(res, "aes128-sha256-x25519") == 0; break;
      case 4: { unsigned int a, b; ok = davici_value_scanf(res, "aes%u-sha%u", &a, &b) == 2 && a == 256 && b == 384; } break;
      case 6: v = davici_get_value(res, &len);
              ok = !strcmp(davici_get_name(res), "blob") && len == sizeof big && v && !memcmp(v, big, len); break;
      case 8: ok = davici_get_level(res) == 0; break;
    }
  }
  echo_ok = ok;
  done++;
}
static void echodumpcb(struct davici_conn *c, int err, const char *name,
                       struct davici_response *res, void *user) {
  (void)c; (void)name; (void)user;
  if (err >= 0 && res) {
    FILE *f = tmpfile();   /* QNX has no fmemopen() */
    if (f) {
      davici_dump(res, "echo", "\n", 0, 2, f);
      rewind(f);
      dumped[fread(dumped, 1, sizeof dumped - 1, f)] = 0;
      fclose(f);
    }
  }
  done++;
}
static int unknown_err = 1;
static void unknowncb(struct davici_conn *c, int err, const char *name,
                      struct davici_response *res, void *user) {
  (void)c; (void)name; (void)user;
  unknown_err = res == NULL ? err : 1;
  done++;
}
static int events, confirms, event_payload_ok = 1;
static void logcb(struct davici_conn *c, int err, const char *name,
                  struct davici_response *res, void *user) {
  (void)c; (void)user;
  if (err < 0 || strcmp(name, "log")) { event_payload_ok = 0; return; }
  if (!res) { confirms++; return; }   /* (un)registration confirmed */
  char want[32], got[32] = "";
  snprintf(want, sizeof want, "message %d", events + 1);
  if (davici_parse(res) != DAVICI_KEY_VALUE || strcmp(davici_get_name(res), "msg") ||
      davici_get_value_str(res, got, sizeof got) < 0 || strcmp(got, want) ||
      davici_parse(res) != DAVICI_END)
    event_payload_ok = 0;
  events++;
}

static struct davici_request *nested_request(void) {
  struct davici_request *r;
  if (davici_new_cmd("echo", &r) < 0) return NULL;
  davici_section_start(r, "conn");
  davici_kvf(r, "remote_addrs", "%s", "127.0.0.2");
  davici_list_start(r, "proposals");
  davici_list_itemf(r, "%s", "aes128-sha256-x25519");
  davici_list_itemf(r, "aes%d-sha%d", 256, 384);
  davici_list_end(r);
  davici_kv(r, "blob", big, sizeof big);
  davici_section_end(r);
  return r;
}

int main(void) {
  QT_BEGIN("davici");
  for (size_t i = 0; i < sizeof big; i++) big[i] = (char)('A' + i % 26);
  char path[64];
  const char *tmp = getenv("TMPDIR");
  snprintf(path, sizeof path, "%s/qnxport-davici-%d.vici", tmp && *tmp && strlen(tmp) < 40 ? tmp : "/tmp", (int)getpid());
  struct sockaddr_un sun = { .sun_family = AF_UNIX };
  snprintf(sun.sun_path, sizeof sun.sun_path, "%s", path);
  unlink(path);
  listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  int lok = listen_fd >= 0 && bind(listen_fd, (struct sockaddr *)&sun, sizeof sun) == 0 &&
            listen(listen_fd, 4) == 0;
  QT_CHECK(lok, "VICI server listening on a UNIX socket");
  if (!lok) QT_END();
  pthread_t th;
  pthread_create(&th, NULL, server_thread, (void *)(intptr_t)1);

  struct davici_conn *c = NULL;
  int r = davici_connect_unix(path, iocb, NULL, &c);
  QT_CHECK(r >= 0 && c, "davici_connect_unix");
  if (r < 0) { unlink(path); QT_END(); }

  struct davici_request *req;
  done = 0;
  davici_new_cmd("version", &req);
  davici_queue(c, req, versioncb, NULL);
  QT_CHECK(run_until(c, &done, 1) == 0, "version command answered");
  QT_CHECK(version_req_ok, "request on the wire: length 9, CMD_REQUEST, name \"version\", no elements");
  printf("  info response: %s\n", version_seen);
  QT_CHECK(!strcmp(version_seen, "daemon=charon version=6.0.6 sysname=QNX"),
           "response parsed: daemon=charon version=6.0.6 sysname=QNX");

  done = 0;
  davici_queue(c, nested_request(), echocb, NULL);
  davici_queue(c, nested_request(), echodumpcb, NULL);
  QT_CHECK(davici_queue_len(c) >= 1, "two requests queued");
  QT_CHECK(run_until(c, &done, 2) == 0, "both echo commands answered (in order)");
  QT_CHECK(echo_ok && echo_elems == 9,
           "echoed message parses back: section, key-value, list of 2 (one scanf'd: 256/384), 3000-byte value, end");
  const char *dwant = "echo {\n  conn {\n    remote_addrs = 127.0.0.2\n    proposals [\n"
                      "      aes128-sha256-x25519\n      aes256-sha384\n    ]\n";
  if (strncmp(dumped, dwant, strlen(dwant))) printf("  info dump: %.300s\n", dumped);
  QT_CHECK(!strncmp(dumped, dwant, strlen(dwant)), "davici_dump: the nested message as text");

  done = 0;
  davici_new_cmd("no-such-command", &req);
  davici_queue(c, req, unknowncb, NULL);
  QT_CHECK(run_until(c, &done, 1) == 0 && unknown_err == -ENOSYS, "unknown command: -ENOSYS");

  davici_register(c, "log", logcb, NULL);
  int want_ev = 2;
  QT_CHECK(run_until(c, &events, want_ev) == 0 && confirms == 1, "event registration confirmed, 2 events received");
  QT_CHECK(event_payload_ok && events == 2, "event payloads: msg=\"message 1\", \"message 2\"");
  davici_unregister(c, "log", logcb, NULL);
  QT_CHECK(run_until(c, &confirms, 2) == 0, "event unregistration confirmed");
  davici_disconnect(c);
  pthread_join(th, NULL);
  close(listen_fd);
  unlink(path);

  /* TCP loopback */
  struct sockaddr_in sin = { .sin_family = AF_INET };
  sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  socklen_t sl = sizeof sin;
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  lok = listen_fd >= 0 && bind(listen_fd, (struct sockaddr *)&sin, sizeof sin) == 0 &&
        listen(listen_fd, 4) == 0 && getsockname(listen_fd, (struct sockaddr *)&sin, &sl) == 0;
  QT_CHECK(lok, "VICI server listening on 127.0.0.1 (TCP)");
  if (lok) {
    pthread_create(&th, NULL, server_thread, (void *)(intptr_t)1);
    r = davici_connect_tcp((struct sockaddr *)&sin, iocb, NULL, &c);
    QT_CHECK(r >= 0, "davici_connect_tcp to 127.0.0.1");
    if (r >= 0) {
      done = 0; version_seen[0] = 0;
      davici_new_cmd("version", &req);
      davici_queue(c, req, versioncb, NULL);
      QT_CHECK(run_until(c, &done, 1) == 0 && !strcmp(version_seen, "daemon=charon version=6.0.6 sysname=QNX"),
               "version command over TCP");
      davici_disconnect(c);
    }
    pthread_join(th, NULL);
    close(listen_fd);
  }
  if (srv_errors) printf("  info server: %d error(s) %s\n", srv_errors, srv_note);
  QT_CHECK(srv_errors == 0, "server saw only well-formed packets");
  QT_END();
}
