/* lighttpd: the daemon started on 127.0.0.1 with a minimal configuration in
 * /tmp; a static file is fetched over HTTP/1.1 and compared byte for byte,
 * its Content-Type and Content-Length checked, a missing file gives 404, a
 * Range request gives 206 with the right slice, and the config test mode
 * (-t) accepts the file.  The server is stopped with SIGTERM at the end. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const char *LIGHTTPD[] = { "/usr/local/sbin/lighttpd", "lighttpd", NULL };
static pid_t run(const char *conf, const char *flag) {
  pid_t p = fork();
  if (p == 0) {
    for (int i = 0; LIGHTTPD[i]; i++) execlp(LIGHTTPD[i], "lighttpd", flag, "-f", conf, (char *)NULL);
    _exit(127);
  }
  return p;
}
static int dial(unsigned short port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in a; memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(fd, (struct sockaddr *)&a, sizeof a) != 0) { close(fd); return -1; }
  struct timeval tv = { 10, 0 }; setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  return fd;
}
/* One request with Connection: close; the whole response into buf. */
static int http(unsigned short port, const char *req, char *buf, int cap) {
  int fd = dial(port); if (fd < 0) return -1;
  (void)!write(fd, req, strlen(req));
  int n = 0, r;
  while (n < cap - 1 && (r = (int)read(fd, buf + n, cap - 1 - n)) > 0) n += r;
  buf[n] = 0; close(fd);
  return n;
}
static int status(const char *resp) { int s = 0; return sscanf(resp, "HTTP/1.%*d %d", &s) == 1 ? s : -1; }
static const char *body(const char *resp) { const char *b = strstr(resp, "\r\n\r\n"); return b ? b + 4 : ""; }
static int has_header(const char *resp, const char *h) {
  const char *end = strstr(resp, "\r\n\r\n");
  for (const char *p = resp; (p = strstr(p, "\r\n")) && (!end || p < end); p += 2)
    if (strncasecmp(p + 2, h, strlen(h)) == 0) return 1;
  return 0;
}

int main(void) {
  QT_BEGIN("lighttpd");
  char dir[64], www[96], conf[128], path[160];
  snprintf(dir, sizeof dir, "/tmp/qnxport-lighttpd-%d", (int)getpid());
  snprintf(www, sizeof www, "%s/www", dir);
  mkdir(dir, 0755); mkdir(www, 0755);
  /* a known file: 4096 bytes of a repeating pattern */
  static char doc[4096];
  for (int i = 0; i < (int)sizeof doc; i++) doc[i] = "lighttpd-on-qnx\n"[i % 16];
  snprintf(path, sizeof path, "%s/hello.txt", www);
  FILE *f = fopen(path, "wb"); fwrite(doc, 1, sizeof doc, f); fclose(f);
  snprintf(path, sizeof path, "%s/index.html", www);
  f = fopen(path, "w"); fputs("<html><body>index</body></html>\n", f); fclose(f);

  unsigned short port = (unsigned short)(18000 + getpid() % 20000);
  snprintf(conf, sizeof conf, "%s/lighttpd.conf", dir);
  f = fopen(conf, "w");
  fprintf(f,
    "server.document-root = \"%s\"\n"
    "server.bind = \"127.0.0.1\"\n"
    "server.port = %u\n"
    "server.errorlog = \"%s/error.log\"\n"
    "server.pid-file = \"%s/lighttpd.pid\"\n"
    "server.upload-dirs = ( \"%s\" )\n"
    "server.feature-flags = ( \"server.h2proto\" => \"disable\" )\n"
    "server.max-worker = 0\n"
    "index-file.names = ( \"index.html\" )\n"
    "mimetype.assign = ( \".txt\" => \"text/plain\", \".html\" => \"text/html\" )\n",
    www, port, dir, dir, dir);
  fclose(f);

  int st = -1; pid_t t = run(conf, "-tt");
  waitpid(t, &st, 0);
  QT_CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 0, "lighttpd -tt accepts the configuration");

  pid_t pid = run(conf, "-D");
  int up = 0;
  for (int i = 0; i < 100 && !up; i++) {
    int fd = dial(port);
    if (fd >= 0) { close(fd); up = 1; break; }
    if (waitpid(pid, &st, WNOHANG) == pid) { printf("  info lighttpd exited early, status 0x%x\n", st); pid = -1; break; }
    usleep(100000);
  }
  QT_CHECK(up, "lighttpd listening on 127.0.0.1");

  static char resp[16384];
  if (up) {
    int n = http(port, "GET /hello.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n", resp, sizeof resp);
    const char *b = body(resp);
    printf("  info GET /hello.txt: %d bytes, status %d\n", n, status(resp));
    QT_CHECK(status(resp) == 200, "GET /hello.txt -> 200");
    QT_CHECK(has_header(resp, "Content-Length: 4096"), "Content-Length: 4096");
    QT_CHECK(has_header(resp, "Content-Type: text/plain"), "Content-Type from mimetype.assign");
    QT_CHECK(n - (int)(b - resp) == (int)sizeof doc && memcmp(b, doc, sizeof doc) == 0, "body identical to the file");

    http(port, "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n", resp, sizeof resp);
    QT_CHECK(status(resp) == 200 && strcmp(body(resp), "<html><body>index</body></html>\n") == 0, "GET / serves index.html");

    http(port, "GET /no-such-file HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n", resp, sizeof resp);
    printf("  info GET /no-such-file: status %d\n", status(resp));
    QT_CHECK(status(resp) == 404, "missing file -> 404");

    http(port, "GET /hello.txt HTTP/1.1\r\nHost: localhost\r\nRange: bytes=16-31\r\nConnection: close\r\n\r\n", resp, sizeof resp);
    b = body(resp);
    QT_CHECK(status(resp) == 206 && strlen(b) == 16 && memcmp(b, doc + 16, 16) == 0, "Range request -> 206 with the right bytes");

    http(port, "HEAD /hello.txt HTTP/1.0\r\n\r\n", resp, sizeof resp);
    QT_CHECK(status(resp) == 200 && body(resp)[0] == 0, "HEAD over HTTP/1.0: 200, no body");
  }
  if (pid > 0) {
    kill(pid, SIGTERM);
    int ok = 0;
    for (int i = 0; i < 50; i++) { if (waitpid(pid, &st, WNOHANG) == pid) { ok = 1; break; } usleep(100000); }
    if (!ok) { kill(pid, SIGKILL); waitpid(pid, &st, 0); }
    QT_CHECK(ok, "lighttpd stops on SIGTERM");
  }
  if (!up || qt_fail_count) {
    snprintf(path, sizeof path, "%s/error.log", dir);
    f = fopen(path, "r"); char line[256];
    while (f && fgets(line, sizeof line, f)) printf("  info log: %s", line);
    if (f) fclose(f);
  }
  char cmd[128]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir); (void)!system(cmd);
  QT_END();
}
