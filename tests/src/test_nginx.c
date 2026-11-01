/* nginx: version and module list (-V), the configuration test (-t) on a
 * good and a broken file, then the server itself on 127.0.0.1 (master and
 * one worker, poll events, in the foreground) with a configuration in /tmp:
 * a static file byte for byte with its Content-Type and Content-Length, a
 * 404, "return" with a body and with a redirect, a PCRE rewrite, gzip
 * (zlib) on request, and the same file over HTTPS (TLS with a self-signed
 * certificate this test makes with OpenSSL). nginx is stopped on every exit
 * path (normal, failed check, signal, overall timeout). */
#include "common.h"
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

static const char *NGINX = "/usr/local/sbin/nginx";
static char dir[96];
static volatile pid_t server = -1;

static void stop_server(void) {
  if (server > 0) {
    pid_t p = server;
    kill(p, SIGTERM);  /* fast shutdown of master and workers */
    for (int i = 0; i < 50; i++) {
      if (waitpid(p, NULL, WNOHANG) == p) { server = -1; return; }
      usleep(100000);
    }
    kill(p, SIGKILL);
    waitpid(p, NULL, 0);
    server = -1;
  }
}
static void cleanup(void) {
  stop_server();
  if (dir[0]) {
    char cmd[160];
    snprintf(cmd, sizeof cmd, "rm -rf %s", dir);
    (void)!system(cmd);
  }
}
static void on_signal(int sig) {
  (void)sig;
  /* exit() runs cleanup() through atexit */
  exit(2);
}

static int run_capture(const char *const argv[], char *out, int cap) {
  int fds[2];
  if (pipe(fds) != 0) return -1;
  pid_t p = fork();
  if (p == 0) {
    dup2(fds[1], 1); dup2(fds[1], 2); close(fds[0]); close(fds[1]);
    execv(argv[0], (char *const *)argv);
    _exit(127);
  }
  close(fds[1]);
  int n = 0, r;
  while (n < cap - 1 && (r = (int)read(fds[0], out + n, cap - 1 - n)) > 0) n += r;
  out[n] = 0;
  close(fds[0]);
  int st = 0;
  waitpid(p, &st, 0);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

static int dial(unsigned short port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in a; memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(fd, (struct sockaddr *)&a, sizeof a) != 0) { close(fd); return -1; }
  struct timeval tv = { 10, 0 };
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
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
static const char *header(const char *resp, const char *h) {
  const char *end = strstr(resp, "\r\n\r\n");
  for (const char *p = resp; (p = strstr(p, "\r\n")) && (!end || p < end); p += 2)
    if (strncasecmp(p + 2, h, strlen(h)) == 0) return p + 2 + strlen(h);
  return NULL;
}
static int header_is(const char *resp, const char *h, const char *v) {
  const char *p = header(resp, h);
  return p && strncmp(p, v, strlen(v)) == 0;
}

/* A self-signed P-256 certificate and its key, as PEM files. */
static int make_cert(const char *crt, const char *key) {
  EVP_PKEY *pk = EVP_EC_gen("P-256");
  X509 *x = X509_new();
  int ok = pk && x && X509_set_version(x, 2) &&
           ASN1_INTEGER_set(X509_get_serialNumber(x), 1) &&
           X509_gmtime_adj(X509_getm_notBefore(x), -60) &&
           X509_gmtime_adj(X509_getm_notAfter(x), 3600);
  X509_NAME *n = ok ? X509_get_subject_name(x) : NULL;
  ok = ok && X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_ASC, (const unsigned char *)"localhost", -1, -1, 0) &&
       X509_set_issuer_name(x, n) && X509_set_pubkey(x, pk) && X509_sign(x, pk, EVP_sha256());
  FILE *f = ok ? fopen(crt, "w") : NULL;
  ok = f && PEM_write_X509(f, x); if (f) fclose(f);
  f = ok ? fopen(key, "w") : NULL;
  ok = f && PEM_write_PrivateKey(f, pk, NULL, NULL, 0, NULL, NULL); if (f) fclose(f);
  X509_free(x); EVP_PKEY_free(pk);
  return ok;
}

static int https_get(unsigned short port, const char *req, char *buf, int cap, char *proto, int plen) {
  SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
  int fd = dial(port), n = -1;
  if (ctx && fd >= 0) {
    SSL *s = SSL_new(ctx);
    SSL_set_fd(s, fd);
    SSL_set_tlsext_host_name(s, "localhost");
    if (SSL_connect(s) == 1) {
      snprintf(proto, plen, "%s %s", SSL_get_version(s), SSL_get_cipher_name(s));
      SSL_write(s, req, (int)strlen(req));
      int r; n = 0;
      while (n < cap - 1 && (r = SSL_read(s, buf + n, cap - 1 - n)) > 0) n += r;
      buf[n] = 0;
    } else {
      ERR_print_errors_fp(stdout);
    }
    SSL_free(s);
  }
  if (fd >= 0) close(fd);
  SSL_CTX_free(ctx);
  return n;
}

static void writef(const char *path, const char *s) {
  FILE *f = fopen(path, "w"); if (f) { fputs(s, f); fclose(f); }
}

int main(void) {
  QT_BEGIN("nginx");
  atexit(cleanup);
  signal(SIGINT, on_signal); signal(SIGTERM, on_signal);
  signal(SIGHUP, on_signal); signal(SIGALRM, on_signal);
  signal(SIGPIPE, SIG_IGN);
  alarm(150);

  static char out[16384];
  const char *const v[] = { NGINX, "-V", NULL };
  int rc = run_capture(v, out, sizeof out);
  QT_CHECK(rc == 0 && strstr(out, "nginx version: nginx/1.30.6"), "nginx -V: version 1.30.6");
  QT_CHECK(strstr(out, "--with-http_ssl_module") && strstr(out, "built with OpenSSL"),
           "nginx -V: built with OpenSSL and the SSL module");
  char *nl = strchr(out, '\n'); if (nl) *nl = 0;
  printf("  info %s\n", out);

  snprintf(dir, sizeof dir, "/tmp/qnxport-nginx-%d", (int)getpid());
  char www[128], conf[160], bad[160], err[160], crt[160], key[160], path[200];
  snprintf(www, sizeof www, "%s/www", dir);
  snprintf(conf, sizeof conf, "%s/nginx.conf", dir);
  snprintf(bad, sizeof bad, "%s/bad.conf", dir);
  snprintf(err, sizeof err, "%s/error.log", dir);
  snprintf(crt, sizeof crt, "%s/server.crt", dir);
  snprintf(key, sizeof key, "%s/server.key", dir);
  mkdir(dir, 0755); mkdir(www, 0755);
  snprintf(path, sizeof path, "%s/logs", dir); mkdir(path, 0755);

  /* a known file: 6000 bytes of a repeating pattern, compressible */
  static char content[6001];
  for (int i = 0; i < 6000; i++) content[i] = "nginx on QNX\n"[i % 13];
  content[6000] = 0;
  snprintf(path, sizeof path, "%s/hello.txt", www); writef(path, content);
  QT_CHECK(make_cert(crt, key), "self-signed certificate written (OpenSSL)");

  unsigned short port = (unsigned short)(18080 + getpid() % 500 * 2), sport = port + 1;
  struct passwd *pw = getpwuid(getuid());
  struct group *gr = getgrgid(getgid());
  static char cfg[4096];
  snprintf(cfg, sizeof cfg,
    "%s%s%s%s%s"
    "worker_processes 1;\n"
    "daemon off;\n"
    "pid %s/nginx.pid;\n"
    "error_log %s info;\n"
    "events { worker_connections 64; }\n"
    "http {\n"
    "  types { text/plain txt; text/html html; }\n"
    "  default_type application/octet-stream;\n"
    "  access_log off;\n"
    "  client_body_temp_path %s/cb; proxy_temp_path %s/px; fastcgi_temp_path %s/fc;\n"
    "  uwsgi_temp_path %s/uw; scgi_temp_path %s/sc;\n"
    "  gzip on; gzip_min_length 100; gzip_types text/plain;\n"
    "  server {\n"
    "    listen 127.0.0.1:%u;\n"
    "    listen 127.0.0.1:%u ssl;\n"
    "    ssl_certificate %s; ssl_certificate_key %s;\n"
    "    root %s;\n"
    "    location = /ping { return 200 \"pong from nginx\\n\"; }\n"
    "    location = /old { return 301 /hello.txt; }\n"
    "    rewrite ^/r/([a-z]+)\\.text$ /$1.txt last;\n"
    "  }\n"
    "}\n",
    pw && gr ? "user " : "", pw && gr ? pw->pw_name : "", pw && gr ? " " : "",
    pw && gr ? gr->gr_name : "", pw && gr ? ";\n" : "",
    dir, err, dir, dir, dir, dir, dir, port, sport, crt, key, www);
  writef(conf, cfg);
  writef(bad, "events { }\nhttp { server { listen 127.0.0.1:1; frobnicate on; } }\n");

  const char *const t1[] = { NGINX, "-t", "-p", dir, "-e", err, "-c", conf, NULL };
  rc = run_capture(t1, out, sizeof out);
  QT_CHECK(rc == 0 && strstr(out, "syntax is ok") && strstr(out, "test is successful"),
           "nginx -t accepts the configuration");
  if (rc != 0) printf("  info %s\n", out);
  const char *const t2[] = { NGINX, "-t", "-p", dir, "-e", err, "-c", bad, NULL };
  rc = run_capture(t2, out, sizeof out);
  QT_CHECK(rc != 0 && strstr(out, "unknown directive \"frobnicate\""),
           "nginx -t rejects an unknown directive");

  pid_t p = fork();
  if (p == 0) {
    execl(NGINX, "nginx", "-p", dir, "-e", err, "-c", conf, (char *)NULL);
    _exit(127);
  }
  server = p;
  static char resp[65536];
  int n = -1;
  for (int i = 0; i < 50 && n <= 0; i++) {
    usleep(200000);
    if (waitpid(p, NULL, WNOHANG) == p) { server = -1; break; }
    n = http(port, "GET /hello.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n", resp, sizeof resp);
  }
  QT_CHECK(n > 0, "nginx is up and answering on 127.0.0.1");
  if (n <= 0) {
    snprintf(path, sizeof path, "cat %s", err); (void)!system(path);
  }
  QT_CHECK(n > 0 && status(resp) == 200 && strcmp(body(resp), content) == 0,
           "GET /hello.txt: 200 and the file byte for byte");
  QT_CHECK(n > 0 && header_is(resp, "Content-Type: ", "text/plain"), "Content-Type: text/plain");
  QT_CHECK(n > 0 && header_is(resp, "Content-Length: ", "6000\r"), "Content-Length: 6000");
  QT_CHECK(n > 0 && header_is(resp, "Server: ", "nginx/1.30.6"), "Server: nginx/1.30.6");

  n = http(port, "GET /nope.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n", resp, sizeof resp);
  QT_CHECK(n > 0 && status(resp) == 404, "GET /nope.txt: 404");

  n = http(port, "GET /ping HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n", resp, sizeof resp);
  QT_CHECK(n > 0 && status(resp) == 200 && strcmp(body(resp), "pong from nginx\n") == 0,
           "return 200 with a body");

  n = http(port, "GET /old HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n", resp, sizeof resp);
  QT_CHECK(n > 0 && status(resp) == 301 && header(resp, "Location: ") &&
               strstr(header(resp, "Location: "), "/hello.txt\r") != NULL,
           "return 301: Location ends in /hello.txt");

  n = http(port, "GET /r/hello.text HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n", resp, sizeof resp);
  QT_CHECK(n > 0 && status(resp) == 200 && strcmp(body(resp), content) == 0,
           "PCRE rewrite /r/hello.text -> /hello.txt");

  n = http(port, "GET /hello.txt HTTP/1.1\r\nHost: localhost\r\nAccept-Encoding: gzip\r\nConnection: close\r\n\r\n",
           resp, sizeof resp);
  const char *b = n > 0 ? body(resp) : "";
  /* chunked: the gzip stream (1f 8b) follows the first chunk-size line */
  const char *gz = strstr(b, "\r\n");
  QT_CHECK(n > 0 && status(resp) == 200 && header_is(resp, "Content-Encoding: ", "gzip") &&
               gz && (unsigned char)gz[2] == 0x1f && (unsigned char)gz[3] == 0x8b,
           "gzip on request: Content-Encoding gzip, gzip stream");
  QT_CHECK(n > 0 && n < 3000, "gzip: compressed response is smaller than the file");

  char proto[128] = "";
  n = https_get(sport, "GET /hello.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n",
                resp, sizeof resp, proto, sizeof proto);
  printf("  info HTTPS: %s\n", proto);
  QT_CHECK(n > 0 && status(resp) == 200 && strcmp(body(resp), content) == 0,
           "HTTPS GET /hello.txt: 200 and the file byte for byte");
  QT_CHECK(strncmp(proto, "TLSv1.3", 7) == 0, "HTTPS: TLS 1.3 negotiated");

  /* graceful stop: SIGQUIT, the master exits after its worker */
  int st = -1;
  if (server > 0) {
    kill(server, SIGQUIT);
    for (int i = 0; i < 100; i++) {
      if (waitpid(server, &st, WNOHANG) == server) { server = -1; break; }
      usleep(100000);
    }
  }
  QT_CHECK(server == -1 && WIFEXITED(st) && WEXITSTATUS(st) == 0, "graceful stop (SIGQUIT): exit 0");
  QT_END();
}
