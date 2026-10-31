/* dnsmasq: run the ported server on loopback (a private port, no upstream
 * servers, no /etc/hosts, configuration on the command line) and query it
 * over UDP and TCP as a resolver would: A records from --address (a whole
 * domain) and --host-record, the matching PTR, a CNAME with its target, a
 * TXT record, and a name it has no data for. */
#include "common.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

static int port;
struct rr { int type; char data[256]; };
struct answer { int ok, rcode, n; struct rr rr[8]; };

static int put_name(unsigned char *p, const char *name) {
  int o = 0;
  while (*name) {
    const char *dot = strchr(name, '.'); int l = dot ? (int)(dot - name) : (int)strlen(name);
    p[o++] = (unsigned char)l; memcpy(p + o, name, l); o += l;
    name += l; if (*name == '.') name++;
  }
  p[o++] = 0;
  return o;
}
/* read a (possibly compressed) name at *off into out; advances *off */
static int get_name(const unsigned char *m, int len, int *off, char *out, int outlen) {
  int o = *off, jumped = 0, n = 0, guard = 0;
  out[0] = 0;
  while (o < len && guard++ < 64) {
    int l = m[o];
    if (l == 0) { o++; break; }
    if ((l & 0xC0) == 0xC0) { if (o + 1 >= len) return -1; if (!jumped) *off = o + 2; jumped = 1; o = ((l & 0x3F) << 8) | m[o + 1]; continue; }
    if (o + 1 + l > len || n + l + 2 > outlen) return -1;
    if (n) out[n++] = '.';
    memcpy(out + n, m + o + 1, l); n += l; out[n] = 0; o += 1 + l;
  }
  if (!jumped) *off = o;
  return 0;
}
static struct answer query(const char *name, int qtype, int tcp) {
  struct answer a; memset(&a, 0, sizeof a);
  unsigned char q[512], r[2048]; int ql = 12;
  memset(q, 0, 12); q[0] = 0x51; q[1] = (unsigned char)(qtype + tcp); q[2] = 0x01; q[5] = 1;   /* id, RD, 1 question */
  ql += put_name(q + ql, name);
  q[ql++] = 0; q[ql++] = (unsigned char)qtype; q[ql++] = 0; q[ql++] = 1;            /* IN */
  struct sockaddr_in sa; memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET; sa.sin_port = htons((unsigned short)port); sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  int s = socket(AF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
  struct timeval tv = { 1, 0 };
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  int rl = -1;
  if (tcp) {
    if (connect(s, (struct sockaddr *)&sa, sizeof sa) == 0) {
      unsigned char lenb[2] = { (unsigned char)(ql >> 8), (unsigned char)ql };
      if (write(s, lenb, 2) == 2 && write(s, q, ql) == ql) {
        int got = 0, want = -1;
        while (want < 0 || got < want + 2) {
          int k = (int)read(s, r + got, sizeof r - got);
          if (k <= 0) break;
          got += k;
          if (got >= 2) want = (r[0] << 8) | r[1];
        }
        if (want >= 0 && got >= want + 2) { memmove(r, r + 2, want); rl = want; }
      }
    }
  } else {
    sendto(s, q, ql, 0, (struct sockaddr *)&sa, sizeof sa);
    rl = (int)recv(s, r, sizeof r, 0);
  }
  close(s);
  if (rl < 12 || r[0] != q[0] || r[1] != q[1] || !(r[2] & 0x80)) return a;
  a.ok = 1; a.rcode = r[3] & 0x0F;
  int qd = (r[4] << 8) | r[5], an = (r[6] << 8) | r[7], off = 12; char tmp[256];
  for (int i = 0; i < qd; i++) { if (get_name(r, rl, &off, tmp, sizeof tmp)) return a; off += 4; }
  for (int i = 0; i < an && a.n < 8 && off + 10 <= rl; i++) {
    if (get_name(r, rl, &off, tmp, sizeof tmp)) break;
    int type = (r[off] << 8) | r[off + 1], rdlen = (r[off + 8] << 8) | r[off + 9];
    off += 10;
    if (off + rdlen > rl) break;
    struct rr *x = &a.rr[a.n++]; x->type = type;
    if (type == 1 && rdlen == 4) inet_ntop(AF_INET, r + off, x->data, sizeof x->data);
    else if (type == 5 || type == 12) { int o2 = off; get_name(r, rl, &o2, x->data, sizeof x->data); }
    else if (type == 16 && rdlen > 0) { int l = r[off]; snprintf(x->data, sizeof x->data, "%.*s", l, (const char *)r + off + 1); }
    off += rdlen;
  }
  return a;
}
static int has(struct answer *a, int type, const char *data) {
  for (int i = 0; i < a->n; i++) if (a->rr[i].type == type && !strcmp(a->rr[i].data, data)) return 1;
  return 0;
}

int main(void) {
  QT_BEGIN("dnsmasq");
  const char *bins[] = { getenv("QNX_PORT_DNSMASQ"), "/usr/local/sbin/dnsmasq", "/data/oss/usr/local/sbin/dnsmasq" };
  const char *bin = NULL;
  for (int i = 0; i < 3 && !bin; i++) if (bins[i] && access(bins[i], X_OK) == 0) bin = bins[i];
  QT_CHECK(bin != NULL, "sbin/dnsmasq installed");
  if (!bin) QT_END();
  port = 20000 + (int)(getpid() % 20000);
  struct passwd *pw = getpwuid(getuid());
  char user[64], portarg[32], logpath[256];
  snprintf(user, sizeof user, "--user=%s", pw ? pw->pw_name : "root");
  snprintf(portarg, sizeof portarg, "--port=%d", port);
  snprintf(logpath, sizeof logpath, "%s/qnxport-dnsmasq-%d.log", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp", (int)getpid());
  printf("  info %s on 127.0.0.1:%d\n", bin, port);
  pid_t pid = fork();
  if (pid == 0) {
    FILE *lf = freopen(logpath, "w", stderr); (void)lf;
    dup2(2, 1);
    execl(bin, "dnsmasq", "--keep-in-foreground", "--conf-file=/dev/null", "--no-resolv", "--no-hosts", "--no-poll",
          portarg, "--listen-address=127.0.0.1", "--bind-interfaces", user, "--pid-file=", "--log-facility=-",
          "--address=/qnx.test/10.1.2.3", "--host-record=box.lan,192.168.7.7", "--cname=alias.lan,box.lan",
          "--txt-record=info.lan,hello from dnsmasq", "--local=/lan/", (char *)NULL);
    _exit(127);
  }
  QT_CHECK(pid > 0, "started");
  struct answer a; memset(&a, 0, sizeof a);
  int alive = 1;
  for (int i = 0; i < 50 && !a.ok && alive; i++) {
    a = query("box.lan", 1, 0);
    int st; if (!a.ok && waitpid(pid, &st, WNOHANG) == pid) alive = 0;
    if (!a.ok) usleep(100000);
  }
  QT_CHECK(a.ok && a.rcode == 0 && has(&a, 1, "192.168.7.7"), "UDP: box.lan A 192.168.7.7 (--host-record)");
  if (a.ok) {
    struct answer b = query("deep.sub.qnx.test", 1, 0);
    QT_CHECK(b.rcode == 0 && has(&b, 1, "10.1.2.3"), "deep.sub.qnx.test A 10.1.2.3 (--address for the domain)");
    struct answer c = query("7.7.168.192.in-addr.arpa", 12, 0);
    QT_CHECK(c.rcode == 0 && has(&c, 12, "box.lan"), "PTR 192.168.7.7 -> box.lan");
    struct answer d = query("alias.lan", 1, 0);
    QT_CHECK(d.rcode == 0 && has(&d, 5, "box.lan") && has(&d, 1, "192.168.7.7"), "alias.lan: CNAME box.lan, then its A record");
    struct answer e = query("info.lan", 16, 0);
    QT_CHECK(e.rcode == 0 && has(&e, 16, "hello from dnsmasq"), "info.lan TXT \"hello from dnsmasq\"");
    struct answer f = query("nothing.lan", 1, 0);
    printf("  info nothing.lan: rcode %d, %d answer(s)\n", f.rcode, f.n);
    QT_CHECK(f.ok && f.n == 0 && f.rcode == 3, "nothing.lan: NXDOMAIN (a --local domain, no upstream asked)");
    struct answer g = query("box.lan", 1, 1);
    QT_CHECK(g.ok && g.rcode == 0 && has(&g, 1, "192.168.7.7"), "TCP: box.lan A 192.168.7.7");
  }
  if (pid > 0) {
    kill(pid, SIGTERM);
    int st = 0; for (int i = 0; i < 50 && waitpid(pid, &st, WNOHANG) == 0; i++) usleep(100000);
  }
  if (!a.ok) {
    FILE *lf = fopen(logpath, "r"); char line[256];
    while (lf && fgets(line, sizeof line, lf)) printf("  info dnsmasq: %s", line);
    if (lf) fclose(lf);
  }
  unlink(logpath);
  QT_END();
}
