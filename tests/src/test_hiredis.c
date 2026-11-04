/* hiredis: the client library against a valkey-server started here on
 * 127.0.0.1 (TCP, a UNIX socket and a TLS port with a self-signed
 * certificate made by this test). Checks: the reply parser on a known RESP
 * buffer, redisFormatCommand's exact bytes, SET/GET of a binary value (%b),
 * INCR, a nil reply, an error reply, pipelining (three appended commands
 * read back in order), RESP3 (HELLO 3, a map reply), the UNIX-socket
 * connection, the async API driven by the poll adapter, and hiredis_ssl:
 * an OpenSSL context trusting the certificate, a TLS connection, SET/GET
 * over it. The server is stopped on every exit path. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/hiredis_ssl.h>
#include <hiredis/adapters/poll.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>

static pid_t server = -1;
static char dir[64];

static void stop_server(void) {
  if (server > 0) { kill(server, SIGKILL); waitpid(server, NULL, 0); server = -1; }
}
static void on_signal(int sig) {
  if (server > 0) kill(server, SIGKILL);
  if (sig == SIGALRM) { static const char m[] = "FAIL hiredis (watchdog: test took too long)\n"; (void)!write(1, m, sizeof m - 1); }
  _exit(2);
}

/* A self-signed certificate (CN=127.0.0.1, CA:TRUE) and its key, as PEM. */
static int make_cert(const char *crt, const char *key) {
  EVP_PKEY *pk = EVP_RSA_gen(2048);
  X509 *x = X509_new();
  if (!pk || !x) return 0;
  ASN1_INTEGER_set(X509_get_serialNumber(x), 1);
  X509_gmtime_adj(X509_getm_notBefore(x), -3600);
  X509_gmtime_adj(X509_getm_notAfter(x), 3600L * 24 * 30);
  X509_set_version(x, 2);
  X509_set_pubkey(x, pk);
  X509_NAME *n = X509_get_subject_name(x);
  X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_ASC, (const unsigned char *)"127.0.0.1", -1, -1, 0);
  X509_set_issuer_name(x, n);
  X509_EXTENSION *ext = X509V3_EXT_conf_nid(NULL, NULL, NID_basic_constraints, "critical,CA:TRUE");
  if (ext) { X509_add_ext(x, ext, -1); X509_EXTENSION_free(ext); }
  if (!X509_sign(x, pk, EVP_sha256())) return 0;
  FILE *f = fopen(crt, "w"); if (!f) return 0; PEM_write_X509(f, x); fclose(f);
  f = fopen(key, "w"); if (!f) return 0; PEM_write_PrivateKey(f, pk, NULL, NULL, 0, NULL, NULL); fclose(f);
  X509_free(x); EVP_PKEY_free(pk);
  return 1;
}

static void start(int port, int tlsport, const char *sock, const char *crt, const char *key) {
  char p[16], tp[16], log[128];
  snprintf(p, sizeof p, "%d", port); snprintf(tp, sizeof tp, "%d", tlsport);
  snprintf(log, sizeof log, "%s/valkey.log", dir);
  server = fork();
  if (server == 0) {
    char *argv[] = { "valkey-server", "--port", p, "--bind", "127.0.0.1", "--dir", dir,
                     "--save", "", "--appendonly", "no", "--protected-mode", "no",
                     "--unixsocket", (char *)sock, "--logfile", log, "--maxclients", "64",
                     "--tls-port", tp, "--tls-cert-file", (char *)crt, "--tls-key-file", (char *)key,
                     "--tls-auth-clients", "no", NULL };
    execv("/usr/local/bin/valkey-server", argv);
    execvp("valkey-server", argv);
    _exit(127);
  }
}

/* async: SET then GET, the GET callback records the value and disconnects */
static char async_got[64]; static int async_set_ok, async_done;
static void on_set(redisAsyncContext *c, void *r, void *priv) {
  (void)c; (void)priv; redisReply *rep = r;
  async_set_ok = rep && rep->type == REDIS_REPLY_STATUS && !strcmp(rep->str, "OK");
}
static void on_get(redisAsyncContext *c, void *r, void *priv) {
  (void)priv; redisReply *rep = r;
  if (rep && rep->type == REDIS_REPLY_STRING) snprintf(async_got, sizeof async_got, "%s", rep->str);
  redisAsyncDisconnect(c);
}
static void on_disconnect(const redisAsyncContext *c, int status) { (void)c; (void)status; async_done = 1; }

int main(void) {
  QT_BEGIN("hiredis");
  snprintf(dir, sizeof dir, "/tmp/qnxport-hiredis-%d", (int)getpid());
  mkdir(dir, 0700);
  atexit(stop_server);
  signal(SIGALRM, on_signal); signal(SIGINT, on_signal); signal(SIGTERM, on_signal);
  signal(SIGSEGV, on_signal); signal(SIGABRT, on_signal); signal(SIGPIPE, SIG_IGN);
  alarm(120);

  /* ---- offline: the parser and the command formatter ---- */
  redisReader *rd = redisReaderCreate();
  const char resp[] = "*3\r\n$3\r\nfoo\r\n:42\r\n$-1\r\n";
  redisReaderFeed(rd, resp, 10);          /* in two pieces: an incomplete reply first */
  void *out = NULL;
  int r1 = redisReaderGetReply(rd, &out);
  QT_CHECK(r1 == REDIS_OK && out == NULL, "reader: an incomplete reply yields nothing yet");
  redisReaderFeed(rd, resp + 10, sizeof resp - 1 - 10);
  redisReaderGetReply(rd, &out);
  redisReply *a = out;
  QT_CHECK(a && a->type == REDIS_REPLY_ARRAY && a->elements == 3 &&
           a->element[0]->type == REDIS_REPLY_STRING && !strcmp(a->element[0]->str, "foo") &&
           a->element[1]->type == REDIS_REPLY_INTEGER && a->element[1]->integer == 42 &&
           a->element[2]->type == REDIS_REPLY_NIL, "reader: array [\"foo\", 42, nil]");
  if (a) freeReplyObject(a);
  redisReaderFree(rd);
  char *fmt = NULL;
  int fl = redisFormatCommand(&fmt, "SET %s %b", "k", "v\0w", (size_t)3);
  QT_CHECK(fl == 29 && fmt && !memcmp(fmt, "*3\r\n$3\r\nSET\r\n$1\r\nk\r\n$3\r\nv\0w\r\n", 29),
           "redisFormatCommand: exact RESP bytes (binary %b)");
  redisFreeCommand(fmt);

  /* ---- a server ---- */
  char crt[128], key[128], sock[128];
  snprintf(crt, sizeof crt, "%s/server.crt", dir); snprintf(key, sizeof key, "%s/server.key", dir);
  snprintf(sock, sizeof sock, "%s/valkey.sock", dir);
  QT_CHECK(make_cert(crt, key), "self-signed certificate for the TLS port");
  int port = 20000 + getpid() % 20000, tlsport = port + 1;
  start(port, tlsport, sock, crt, key);
  struct timeval tv = { 2, 0 };
  redisContext *c = NULL;
  for (int i = 0; i < 150; i++) {
    c = redisConnectWithTimeout("127.0.0.1", port, tv);
    if (c && !c->err) { redisReply *p = redisCommand(c, "PING"); int ok = p && p->type == REDIS_REPLY_STATUS && !strcmp(p->str, "PONG"); if (p) freeReplyObject(p); if (ok) break; }
    if (c) { redisFree(c); c = NULL; }
    int st; if (waitpid(server, &st, WNOHANG) == server) { printf("  info valkey-server exited, status 0x%x\n", st); server = -1; break; }
    usleep(100000);
  }
  QT_CHECK(c != NULL, "redisConnect to valkey-server on 127.0.0.1, PING -> PONG");
  if (c) {
    redisReply *r = redisCommand(c, "SET %s %b", "bin", "a\0b\r\nc", (size_t)6);
    QT_CHECK(r && r->type == REDIS_REPLY_STATUS && !strcmp(r->str, "OK"), "SET of a binary value");
    freeReplyObject(r);
    r = redisCommand(c, "GET bin");
    QT_CHECK(r && r->type == REDIS_REPLY_STRING && r->len == 6 && !memcmp(r->str, "a\0b\r\nc", 6), "GET returns the 6 binary bytes");
    freeReplyObject(r);
    redisCommand(c, "SET n 41");
    r = redisCommand(c, "INCR n");
    QT_CHECK(r && r->type == REDIS_REPLY_INTEGER && r->integer == 42, "INCR: 42");
    freeReplyObject(r);
    r = redisCommand(c, "GET nosuchkey");
    QT_CHECK(r && r->type == REDIS_REPLY_NIL, "GET of a missing key: nil");
    freeReplyObject(r);
    r = redisCommand(c, "INCR bin");
    QT_CHECK(r && r->type == REDIS_REPLY_ERROR && !strncmp(r->str, "ERR", 3), "INCR of a non-integer: error reply");
    freeReplyObject(r);
    redisAppendCommand(c, "RPUSH l a b c");
    redisAppendCommand(c, "LLEN l");
    redisAppendCommand(c, "LRANGE l 0 -1");
    redisReply *p1 = NULL, *p2 = NULL, *p3 = NULL;
    redisGetReply(c, (void **)&p1); redisGetReply(c, (void **)&p2); redisGetReply(c, (void **)&p3);
    QT_CHECK(p1 && p1->integer == 3 && p2 && p2->integer == 3 && p3 && p3->elements == 3 &&
             !strcmp(p3->element[2]->str, "c"), "pipelining: three commands, replies in order");
    freeReplyObject(p1); freeReplyObject(p2); freeReplyObject(p3);
    r = redisCommand(c, "HELLO 3");
    int hello = r && r->type == REDIS_REPLY_MAP; freeReplyObject(r);
    redisCommand(c, "HSET h f1 v1 f2 v2");
    r = redisCommand(c, "HGETALL h");
    QT_CHECK(hello && r && r->type == REDIS_REPLY_MAP && r->elements == 4, "RESP3: HELLO 3, HGETALL is a map reply");
    freeReplyObject(r);
    redisFree(c);
  }

  redisContext *u = redisConnectUnix(sock);
  redisReply *ur = (u && !u->err) ? redisCommand(u, "GET n") : NULL;
  QT_CHECK(ur && ur->type == REDIS_REPLY_STRING && !strcmp(ur->str, "42"), "redisConnectUnix: GET over the UNIX socket");
  if (ur) freeReplyObject(ur);
  if (u) redisFree(u);

  /* async API on the poll adapter */
  redisAsyncContext *ac = redisAsyncConnect("127.0.0.1", port);
  if (ac && !ac->err) {
    redisPollAttach(ac);
    redisAsyncSetDisconnectCallback(ac, on_disconnect);
    redisAsyncCommand(ac, on_set, NULL, "SET async %s", "value-from-async");
    redisAsyncCommand(ac, on_get, NULL, "GET async");
    for (int i = 0; i < 2000 && !async_done; i++) redisPollTick(ac, 0.01);
  }
  QT_CHECK(async_set_ok && !strcmp(async_got, "value-from-async") && async_done,
           "async API (poll adapter): SET and GET callbacks, then disconnect");

  /* TLS */
  redisInitOpenSSL();
  redisSSLContextError se = REDIS_SSL_CTX_NONE;
  redisSSLContext *ssl = redisCreateSSLContext(crt, NULL, NULL, NULL, "127.0.0.1", &se);
  QT_CHECK(ssl != NULL, "redisCreateSSLContext trusting the self-signed certificate");
  if (ssl) {
    redisContext *t = redisConnectWithTimeout("127.0.0.1", tlsport, tv);
    int ok = t && !t->err && redisInitiateSSLWithContext(t, ssl) == REDIS_OK;
    if (!ok) printf("  info TLS: %s\n", t ? t->errstr : "no context");
    QT_CHECK(ok, "TLS handshake with valkey-server's tls-port");
    if (ok) {
      redisReply *r = redisCommand(t, "SET tls %s", "over-tls");
      if (r) freeReplyObject(r);
      r = redisCommand(t, "GET tls");
      QT_CHECK(r && r->type == REDIS_REPLY_STRING && !strcmp(r->str, "over-tls"), "SET/GET over TLS");
      if (r) freeReplyObject(r);
    }
    if (t) redisFree(t);
    redisFreeSSLContext(ssl);
  }

  stop_server();
  if (qt_fail_count) {
    char path[160]; snprintf(path, sizeof path, "%s/valkey.log", dir);
    FILE *f = fopen(path, "r"); char line[512]; int n = 0;
    while (f && fgets(line, sizeof line, f)) if (n++ < 40) printf("  info log: %s", line);
    if (f) fclose(f);
  }
  char cmd[128]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir); (void)!system(cmd);
  QT_END();
}
