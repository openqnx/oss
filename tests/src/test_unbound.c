/* unbound: (1) libunbound answering from local zones it is given -- static
 * zone data, an NXDOMAIN inside the static zone, and a hosts file -- with no
 * network at all; (2) the unbound daemon serving a local zone on 127.0.0.1:
 * a raw DNS query over UDP gets the right A record, and libunbound,
 * forwarding to that daemon, resolves the name and a TXT record through it. */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unbound.h>

static int a_is(struct ub_result *r, const char *dotted) {
  struct in_addr want; inet_pton(AF_INET, dotted, &want);
  return r && r->havedata && r->data[0] && r->len[0] == 4 && memcmp(r->data[0], &want, 4) == 0;
}
static void show(const char *what, int rc, struct ub_result *r) {
  if (rc) { printf("  info %s: %s\n", what, ub_strerror(rc)); return; }
  char ip[32] = "-";
  if (r->havedata && r->len[0] == 4) inet_ntop(AF_INET, r->data[0], ip, sizeof ip);
  printf("  info %s: rcode %d havedata %d nxdomain %d first A %s\n", what, r->rcode, r->havedata, r->nxdomain, ip);
}
static void wr(const char *path, const char *text) { FILE *f = fopen(path, "w"); if (f) { fputs(text, f); fclose(f); } }

/* Hand-built DNS query; returns the 4 bytes of the single A answer in out. */
static int raw_query(unsigned short port, const char *name, uint16_t id, unsigned char out[4]) {
  unsigned char q[256], r[512]; int n = 12;
  memset(q, 0, 12); q[0] = id >> 8; q[1] = id & 0xff; q[2] = 0x01; q[5] = 1;
  for (const char *p = name; *p;) {
    const char *dot = strchr(p, '.'); int l = dot ? (int)(dot - p) : (int)strlen(p);
    q[n++] = (unsigned char)l; memcpy(q + n, p, l); n += l; p += l; if (*p == '.') p++;
  }
  q[n++] = 0; q[n++] = 0; q[n++] = 1; q[n++] = 0; q[n++] = 1;
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  struct timeval tv = { 0, 200000 }; setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  struct sockaddr_in a; memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  sendto(fd, q, n, 0, (struct sockaddr *)&a, sizeof a);
  int got = (int)recv(fd, r, sizeof r, 0);
  close(fd);
  if (got < 12 + 4 || r[0] != q[0] || r[1] != q[1]) return -1;
  int rcode = r[3] & 0x0f, an = r[6] << 8 | r[7];
  if (rcode != 0 || an != 1) return rcode ? rcode : -2;
  memcpy(out, r + got - 4, 4);   /* no additional section: the A rdata ends the packet */
  return 0;
}

int main(void) {
  QT_BEGIN("unbound");
  int rc; struct ub_result *r = NULL;
  char dir[64]; snprintf(dir, sizeof dir, "/tmp/qnxport-unbound-%d", (int)getpid());
  mkdir(dir, 0755);
  char path[128];

  /* ---- 1. libunbound, local data only ---- */
  struct ub_ctx *ctx = ub_ctx_create();
  QT_CHECK(ctx != NULL, "ub_ctx_create");
  ub_ctx_set_option(ctx, "do-ip6:", "no");
  /* options and the hosts file go in before the context is finalized
     (the first zone/data call or resolve finalizes it) */
  snprintf(path, sizeof path, "%s/hosts", dir);
  wr(path, "# test hosts file\n10.11.12.13  ecu-gw.vehicle.lan ecu-gw\n");
  rc = ub_ctx_hosts(ctx, path);
  QT_CHECK(rc == 0, "ub_ctx_hosts reads a hosts file");
  QT_CHECK(ub_ctx_zone_add(ctx, "qnx.test.", "static") == 0, "add static local zone qnx.test.");
  QT_CHECK(ub_ctx_data_add(ctx, "host.qnx.test. 300 IN A 192.0.2.7") == 0, "add A record");
  QT_CHECK(ub_ctx_data_add(ctx, "host.qnx.test. 300 IN TXT \"port-test\"") == 0, "add TXT record");
  rc = ub_resolve(ctx, "host.qnx.test", 1, 1, &r); show("host.qnx.test A", rc, r);
  QT_CHECK(rc == 0 && a_is(r, "192.0.2.7") && r->rcode == 0, "A record from local data");
  if (!rc) ub_resolve_free(r);
  rc = ub_resolve(ctx, "host.qnx.test", 16, 1, &r);
  QT_CHECK(rc == 0 && r->havedata && r->len[0] == 10 && r->data[0][0] == 9 && memcmp(r->data[0] + 1, "port-test", 9) == 0,
           "TXT record from local data");
  if (!rc) ub_resolve_free(r);
  rc = ub_resolve(ctx, "missing.qnx.test", 1, 1, &r); show("missing.qnx.test A", rc, r);
  QT_CHECK(rc == 0 && r->nxdomain && !r->havedata && r->rcode == 3, "NXDOMAIN inside the static zone");
  if (!rc) ub_resolve_free(r);
  rc = ub_resolve(ctx, "ecu-gw.vehicle.lan", 1, 1, &r); show("ecu-gw.vehicle.lan A", rc, r);
  QT_CHECK(rc == 0 && a_is(r, "10.11.12.13"), "name from the hosts file");
  if (!rc) ub_resolve_free(r);
  ub_ctx_delete(ctx);

  /* ---- 2. the unbound daemon on loopback ----
     (not under test.: that is one of unbound's default empty zones, RFC 6761,
     so libunbound answers NXDOMAIN for it without asking the daemon) */
  unsigned short port = (unsigned short)(25000 + getpid() % 20000);
  char conf[1400];
  snprintf(conf, sizeof conf,
    "server:\n  verbosity: 1\n  interface: 127.0.0.1\n  port: %u\n  do-ip6: no\n  do-tcp: yes\n"
    "  do-daemonize: no\n  username: \"\"\n  chroot: \"\"\n  directory: \"%s\"\n"
    "  pidfile: \"%s/unbound.pid\"\n  use-syslog: no\n  logfile: \"%s/unbound.log\"\n"
    "  num-threads: 1\n  outgoing-range: 32\n  num-queries-per-thread: 16\n  incoming-num-tcp: 4\n  outgoing-num-tcp: 2\n"
    "  so-reuseport: no\n  so-sndbuf: 0\n  so-rcvbuf: 0\n  module-config: \"iterator\"\n  access-control: 127.0.0.0/8 allow\n"
    "  local-zone: \"vehicle.example.\" static\n"
    "  local-data: \"ivi.vehicle.example. 600 IN A 198.51.100.9\"\n"
    "  local-data: 'ivi.vehicle.example. 600 IN TXT \"served-by-unbound\"'\n"
    "remote-control:\n  control-enable: no\n", port, dir, dir, dir);
  snprintf(path, sizeof path, "%s/unbound.conf", dir);
  wr(path, conf);
  pid_t pid = fork();
  if (pid == 0) {
    execl("/usr/local/sbin/unbound", "unbound", "-d", "-c", path, (char *)NULL);
    execlp("unbound", "unbound", "-d", "-c", path, (char *)NULL);
    _exit(127);
  }
  unsigned char ip[4]; int qr = -1;
  for (int i = 0; i < 100 && qr != 0; i++) {
    qr = raw_query(port, "ivi.vehicle.example", (uint16_t)(0x4000 + i), ip);
    if (qr != 0) {
      int st; if (waitpid(pid, &st, WNOHANG) == pid) { printf("  info unbound exited, status 0x%x\n", st); pid = -1; break; }
      usleep(100000);
    }
  }
  if (qr == 0) printf("  info daemon answered %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
  else printf("  info no answer from the daemon (%d)\n", qr);
  QT_CHECK(qr == 0 && ip[0] == 198 && ip[1] == 51 && ip[2] == 100 && ip[3] == 9, "unbound daemon answers a UDP query on 127.0.0.1");
  unsigned char ip2[4];
  QT_CHECK(qr == 0 && raw_query(port, "nothere.vehicle.example", 0x5151, ip2) == 3, "daemon returns NXDOMAIN for an unknown name");

  ctx = ub_ctx_create();
  char fwd[48]; snprintf(fwd, sizeof fwd, "127.0.0.1@%u", port);
  ub_ctx_set_option(ctx, "do-not-query-localhost:", "no");
  ub_ctx_set_option(ctx, "do-ip6:", "no");
  QT_CHECK(ub_ctx_set_fwd(ctx, fwd) == 0, "libunbound forwards to the daemon");
  if (qr == 0) {
    rc = ub_resolve(ctx, "ivi.vehicle.example", 1, 1, &r); show("ivi.vehicle.example A via daemon", rc, r);
    QT_CHECK(rc == 0 && a_is(r, "198.51.100.9"), "A record resolved through the daemon");
    if (!rc) ub_resolve_free(r);
    rc = ub_resolve(ctx, "ivi.vehicle.example", 16, 1, &r);
    QT_CHECK(rc == 0 && r->havedata && r->len[0] == 18 && memcmp(r->data[0] + 1, "served-by-unbound", 17) == 0,
             "TXT record resolved through the daemon");
    if (!rc) ub_resolve_free(r);
  } else QT_CHECK(0, "resolution through the daemon (daemon not up)");
  ub_ctx_delete(ctx);

  if (pid > 0) { kill(pid, SIGTERM); waitpid(pid, NULL, 0); }
  if (qr != 0) {   /* show why */
    snprintf(path, sizeof path, "%s/unbound.log", dir);
    FILE *f = fopen(path, "r"); char line[256];
    while (f && fgets(line, sizeof line, f)) printf("  info log: %s", line);
    if (f) fclose(f);
  }
  char cmd[128]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir); (void)!system(cmd);
  QT_END();
}
