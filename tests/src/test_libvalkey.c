/* libvalkey: the client library against a valkey-server started here on
 * 127.0.0.1 (TCP, a UNIX socket and a TLS port with a self-signed
 * certificate made by this test). Checks: the reply parser on a known RESP
 * buffer, valkeyFormatCommand's exact bytes, SET/GET of a binary value (%b),
 * INCR, a nil reply, an error reply, pipelining (three appended commands
 * read back in order), RESP3 (HELLO 3, a map reply), the UNIX-socket
 * connection, the async API driven by the poll adapter, libvalkey_tls: an
 * OpenSSL context trusting the certificate, a TLS connection, SET/GET over
 * it; and the cluster client: a second server in cluster mode owning all
 * 16384 slots, CLUSTER KEYSLOT's known answer (foo -> 12182), and
 * valkeyClusterCommand routing SET/GET of 20 keys by slot.
 * The servers are stopped on every exit path. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <valkey/valkey.h>
#include <valkey/async.h>
#include <valkey/cluster.h>
#include <valkey/tls.h>
#include <valkey/adapters/poll.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>

static pid_t server = -1, cserver = -1;
static char dir[64];

static void stop_server(void) {
  if (server > 0) { kill(server, SIGKILL); waitpid(server, NULL, 0); server = -1; }
  if (cserver > 0) { kill(cserver, SIGKILL); waitpid(cserver, NULL, 0); cserver = -1; }
}
static void on_signal(int sig) {
  if (server > 0) kill(server, SIGKILL);
  if (cserver > 0) kill(cserver, SIGKILL);
  if (sig == SIGALRM) { static const char m[] = "FAIL libvalkey (watchdog: test took too long)\n"; (void)!write(1, m, sizeof m - 1); }
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

/* a cluster-mode server: its own directory for nodes.conf */
static void start_cluster(int port) {
  char p[16], cdir[96], log[128];
  snprintf(p, sizeof p, "%d", port);
  snprintf(cdir, sizeof cdir, "%s/cluster", dir); mkdir(cdir, 0700);
  snprintf(log, sizeof log, "%s/valkey.log", cdir);
  cserver = fork();
  if (cserver == 0) {
    char *argv[] = { "valkey-server", "--port", p, "--bind", "127.0.0.1", "--dir", cdir,
                     "--save", "", "--appendonly", "no", "--protected-mode", "no",
                     "--cluster-enabled", "yes", "--cluster-config-file", "nodes.conf",
                     "--logfile", log, "--maxclients", "64", NULL };
    execv("/usr/local/bin/valkey-server", argv);
    execvp("valkey-server", argv);
    _exit(127);
  }
}

/* async: SET then GET, the GET callback records the value and disconnects */
static char async_got[64]; static int async_set_ok, async_done;
static void on_set(valkeyAsyncContext *c, void *r, void *priv) {
  (void)c; (void)priv; valkeyReply *rep = r;
  async_set_ok = rep && rep->type == VALKEY_REPLY_STATUS && !strcmp(rep->str, "OK");
}
static void on_get(valkeyAsyncContext *c, void *r, void *priv) {
  (void)priv; valkeyReply *rep = r;
  if (rep && rep->type == VALKEY_REPLY_STRING) snprintf(async_got, sizeof async_got, "%s", rep->str);
  valkeyAsyncDisconnect(c);
}
static void on_disconnect(const valkeyAsyncContext *c, int status) { (void)c; (void)status; async_done = 1; }

int main(void) {
  QT_BEGIN("libvalkey");
  snprintf(dir, sizeof dir, "/tmp/qnxport-libvalkey-%d", (int)getpid());
  mkdir(dir, 0700);
  atexit(stop_server);
  signal(SIGALRM, on_signal); signal(SIGINT, on_signal); signal(SIGTERM, on_signal);
  signal(SIGSEGV, on_signal); signal(SIGABRT, on_signal); signal(SIGPIPE, SIG_IGN);
  alarm(120);

  /* ---- offline: the parser and the command formatter ---- */
  valkeyReader *rd = valkeyReaderCreate();
  const char resp[] = "*3\r\n$3\r\nfoo\r\n:42\r\n$-1\r\n";
  valkeyReaderFeed(rd, resp, 10);          /* in two pieces: an incomplete reply first */
  void *out = NULL;
  int r1 = valkeyReaderGetReply(rd, &out);
  QT_CHECK(r1 == VALKEY_OK && out == NULL, "reader: an incomplete reply yields nothing yet");
  valkeyReaderFeed(rd, resp + 10, sizeof resp - 1 - 10);
  valkeyReaderGetReply(rd, &out);
  valkeyReply *a = out;
  QT_CHECK(a && a->type == VALKEY_REPLY_ARRAY && a->elements == 3 &&
           a->element[0]->type == VALKEY_REPLY_STRING && !strcmp(a->element[0]->str, "foo") &&
           a->element[1]->type == VALKEY_REPLY_INTEGER && a->element[1]->integer == 42 &&
           a->element[2]->type == VALKEY_REPLY_NIL, "reader: array [\"foo\", 42, nil]");
  if (a) freeReplyObject(a);
  valkeyReaderFree(rd);
  char *fmt = NULL;
  int fl = valkeyFormatCommand(&fmt, "SET %s %b", "k", "v\0w", (size_t)3);
  QT_CHECK(fl == 29 && fmt && !memcmp(fmt, "*3\r\n$3\r\nSET\r\n$1\r\nk\r\n$3\r\nv\0w\r\n", 29),
           "valkeyFormatCommand: exact RESP bytes (binary %b)");
  valkeyFreeCommand(fmt);

  /* ---- a server ---- */
  char crt[128], key[128], sock[128];
  snprintf(crt, sizeof crt, "%s/server.crt", dir); snprintf(key, sizeof key, "%s/server.key", dir);
  snprintf(sock, sizeof sock, "%s/valkey.sock", dir);
  QT_CHECK(make_cert(crt, key), "self-signed certificate for the TLS port");
  int port = 20000 + getpid() % 20000, tlsport = port + 1;
  start(port, tlsport, sock, crt, key);
  struct timeval tv = { 2, 0 };
  valkeyContext *c = NULL;
  for (int i = 0; i < 150; i++) {
    c = valkeyConnectWithTimeout("127.0.0.1", port, tv);
    if (c && !c->err) { valkeyReply *p = valkeyCommand(c, "PING"); int ok = p && p->type == VALKEY_REPLY_STATUS && !strcmp(p->str, "PONG"); if (p) freeReplyObject(p); if (ok) break; }
    if (c) { valkeyFree(c); c = NULL; }
    int st; if (waitpid(server, &st, WNOHANG) == server) { printf("  info valkey-server exited, status 0x%x\n", st); server = -1; break; }
    usleep(100000);
  }
  QT_CHECK(c != NULL, "valkeyConnect to valkey-server on 127.0.0.1, PING -> PONG");
  if (c) {
    valkeyReply *r = valkeyCommand(c, "SET %s %b", "bin", "a\0b\r\nc", (size_t)6);
    QT_CHECK(r && r->type == VALKEY_REPLY_STATUS && !strcmp(r->str, "OK"), "SET of a binary value");
    freeReplyObject(r);
    r = valkeyCommand(c, "GET bin");
    QT_CHECK(r && r->type == VALKEY_REPLY_STRING && r->len == 6 && !memcmp(r->str, "a\0b\r\nc", 6), "GET returns the 6 binary bytes");
    freeReplyObject(r);
    valkeyCommand(c, "SET n 41");
    r = valkeyCommand(c, "INCR n");
    QT_CHECK(r && r->type == VALKEY_REPLY_INTEGER && r->integer == 42, "INCR: 42");
    freeReplyObject(r);
    r = valkeyCommand(c, "GET nosuchkey");
    QT_CHECK(r && r->type == VALKEY_REPLY_NIL, "GET of a missing key: nil");
    freeReplyObject(r);
    r = valkeyCommand(c, "INCR bin");
    QT_CHECK(r && r->type == VALKEY_REPLY_ERROR && !strncmp(r->str, "ERR", 3), "INCR of a non-integer: error reply");
    freeReplyObject(r);
    valkeyAppendCommand(c, "RPUSH l a b c");
    valkeyAppendCommand(c, "LLEN l");
    valkeyAppendCommand(c, "LRANGE l 0 -1");
    valkeyReply *p1 = NULL, *p2 = NULL, *p3 = NULL;
    valkeyGetReply(c, (void **)&p1); valkeyGetReply(c, (void **)&p2); valkeyGetReply(c, (void **)&p3);
    QT_CHECK(p1 && p1->integer == 3 && p2 && p2->integer == 3 && p3 && p3->elements == 3 &&
             !strcmp(p3->element[2]->str, "c"), "pipelining: three commands, replies in order");
    freeReplyObject(p1); freeReplyObject(p2); freeReplyObject(p3);
    r = valkeyCommand(c, "HELLO 3");
    int hello = r && r->type == VALKEY_REPLY_MAP; freeReplyObject(r);
    valkeyCommand(c, "HSET h f1 v1 f2 v2");
    r = valkeyCommand(c, "HGETALL h");
    QT_CHECK(hello && r && r->type == VALKEY_REPLY_MAP && r->elements == 4, "RESP3: HELLO 3, HGETALL is a map reply");
    freeReplyObject(r);
    valkeyFree(c);
  }

  valkeyContext *u = valkeyConnectUnix(sock);
  valkeyReply *ur = (u && !u->err) ? valkeyCommand(u, "GET n") : NULL;
  QT_CHECK(ur && ur->type == VALKEY_REPLY_STRING && !strcmp(ur->str, "42"), "valkeyConnectUnix: GET over the UNIX socket");
  if (ur) freeReplyObject(ur);
  if (u) valkeyFree(u);

  /* async API on the poll adapter */
  valkeyAsyncContext *ac = valkeyAsyncConnect("127.0.0.1", port);
  if (ac && !ac->err) {
    valkeyPollAttach(ac);
    valkeyAsyncSetDisconnectCallback(ac, on_disconnect);
    valkeyAsyncCommand(ac, on_set, NULL, "SET async %s", "value-from-async");
    valkeyAsyncCommand(ac, on_get, NULL, "GET async");
    for (int i = 0; i < 2000 && !async_done; i++) valkeyPollTick(ac, 0.01);
  }
  QT_CHECK(async_set_ok && !strcmp(async_got, "value-from-async") && async_done,
           "async API (poll adapter): SET and GET callbacks, then disconnect");

  /* TLS */
  valkeyInitOpenSSL();
  valkeyTLSContextError se = VALKEY_TLS_CTX_NONE;
  valkeyTLSContext *ssl = valkeyCreateTLSContext(crt, NULL, NULL, NULL, "127.0.0.1", &se);
  QT_CHECK(ssl != NULL, "valkeyCreateTLSContext trusting the self-signed certificate");
  if (ssl) {
    valkeyContext *t = valkeyConnectWithTimeout("127.0.0.1", tlsport, tv);
    int ok = t && !t->err && valkeyInitiateTLSWithContext(t, ssl) == VALKEY_OK;
    if (!ok) printf("  info TLS: %s\n", t ? t->errstr : "no context");
    QT_CHECK(ok, "TLS handshake with valkey-server's tls-port");
    if (ok) {
      valkeyReply *r = valkeyCommand(t, "SET tls %s", "over-tls");
      if (r) freeReplyObject(r);
      r = valkeyCommand(t, "GET tls");
      QT_CHECK(r && r->type == VALKEY_REPLY_STRING && !strcmp(r->str, "over-tls"), "SET/GET over TLS");
      if (r) freeReplyObject(r);
    }
    if (t) valkeyFree(t);
    valkeyFreeTLSContext(ssl);
  }

  /* cluster: one node owning every slot */
  int cport = port + 2;
  start_cluster(cport);
  valkeyContext *cc0 = NULL;
  for (int i = 0; i < 150 && !cc0; i++) {
    cc0 = valkeyConnectWithTimeout("127.0.0.1", cport, tv);
    if (cc0 && cc0->err) { valkeyFree(cc0); cc0 = NULL; usleep(100000); }
  }
  int cluster_ok = 0;
  if (cc0) {
    valkeyReply *r = valkeyCommand(cc0, "CLUSTER ADDSLOTSRANGE 0 16383");
    QT_CHECK(r && r->type == VALKEY_REPLY_STATUS, "CLUSTER ADDSLOTSRANGE 0 16383");
    if (r) freeReplyObject(r);
    r = valkeyCommand(cc0, "CLUSTER KEYSLOT foo");
    QT_CHECK(r && r->type == VALKEY_REPLY_INTEGER && r->integer == 12182, "CLUSTER KEYSLOT foo = 12182");
    if (r) freeReplyObject(r);
    for (int i = 0; i < 100 && !cluster_ok; i++) {
      r = valkeyCommand(cc0, "CLUSTER INFO");
      cluster_ok = r && r->type == VALKEY_REPLY_STRING && strstr(r->str, "cluster_state:ok");
      if (r) freeReplyObject(r);
      if (!cluster_ok) usleep(100000);
    }
    valkeyFree(cc0);
  }
  QT_CHECK(cluster_ok, "cluster-mode valkey-server reaches cluster_state:ok");
  if (cluster_ok) {
    char addr[32]; snprintf(addr, sizeof addr, "127.0.0.1:%d", cport);
    valkeyClusterContext *cc = valkeyClusterConnectWithTimeout(addr, tv);
    QT_CHECK(cc && !cc->err, "valkeyClusterConnect fetched the slot map");
    if (cc && !cc->err) {
      valkeyClusterNodeIterator it; int nodes = 0;
      valkeyClusterInitNodeIterator(&it, cc);
      while (valkeyClusterNodeNext(&it)) nodes++;
      QT_CHECK(nodes == 1, "the cluster has one node");
      int good = 0;
      for (int i = 0; i < 20; i++) {
        char k[16], v[16]; snprintf(k, sizeof k, "key:%d", i); snprintf(v, sizeof v, "val%d", i * 7);
        valkeyReply *r = valkeyClusterCommand(cc, "SET %s %s", k, v);
        if (r) freeReplyObject(r);
        r = valkeyClusterCommand(cc, "GET %s", k);
        if (r && r->type == VALKEY_REPLY_STRING && !strcmp(r->str, v)) good++;
        if (r) freeReplyObject(r);
      }
      QT_CHECK(good == 20, "valkeyClusterCommand: SET/GET of 20 keys routed by slot");
    }
    if (cc) valkeyClusterFree(cc);
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
