/* yubico-c-client (libykclient): OTP validation against a validation
 * server run here on 127.0.0.1 (a forked HTTP responder that speaks the
 * YubiCloud 2.0 protocol and signs its answers with HMAC-SHA1 under the
 * client's API key). Checks: status=OK with a valid signature and the OTP
 * and nonce echoed -> YKCLIENT_OK; status=REPLAYED_OTP -> REPLAYED_OTP; a
 * forged signature -> BAD_SERVER_SIGNATURE; a different OTP echoed ->
 * HMAC_ERROR; the request carried id=, nonce=, otp= and a request
 * signature h=; ykclient_strerror's text; and the ykclient tool verifying
 * through the same server (exit status 0). */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ykclient.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

static const unsigned char KEY[20] = "0123456789abcdefghij";
static const char *OTP = "ccccccbchvthlivuitriujjifivbvtrjkjfirllluurfr";
static pid_t srv = -1;
static void stop(void) { if (srv > 0) { kill(srv, SIGKILL); waitpid(srv, NULL, 0); srv = -1; } }
static void on_alarm(int s) { (void)s; if (srv > 0) kill(srv, SIGKILL); static const char m[] = "FAIL yubico-c-client (watchdog)\n"; (void)!write(1, m, sizeof m - 1); _exit(2); }

/* query parameter value */
static void qparam(const char *req, const char *name, char *out, size_t cap) {
  char pat[32]; snprintf(pat, sizeof pat, "%s=", name); out[0] = 0;
  for (const char *p = req; (p = strstr(p, pat)); p++)
    if (p[-1] == '?' || p[-1] == '&') {
      p += strlen(pat); size_t n = strcspn(p, "& \r\n"); if (n >= cap) n = cap - 1;
      memcpy(out, p, n); out[n] = 0; return;
    }
}
/* the responder: request i gets answer mode[i] */
static void serve(int ls) {
  const char *mode[] = { "ok", "replayed", "forged", "otherotp", "ok", NULL };
  for (int i = 0; mode[i]; i++) {
    int c = accept(ls, NULL, NULL); if (c < 0) _exit(1);
    char req[4096]; int n = 0, r;
    while (n < (int)sizeof req - 1 && (r = (int)read(c, req + n, sizeof req - 1 - n)) > 0) { n += r; req[n] = 0; if (strstr(req, "\r\n\r\n")) break; }
    req[n] = 0;
    char nonce[128], otp[128], id[16], h[128];
    qparam(req, "nonce", nonce, sizeof nonce); qparam(req, "otp", otp, sizeof otp);
    qparam(req, "id", id, sizeof id); qparam(req, "h", h, sizeof h);
    const char *status = !strcmp(mode[i], "replayed") ? "REPLAYED_OTP" : "OK";
    if (!strcmp(mode[i], "otherotp")) otp[0] = otp[0] == 'c' ? 'd' : 'c';
    /* a request without id/nonce/otp/h is answered MISSING_PARAMETER */
    if (!id[0] || !nonce[0] || !otp[0] || !h[0]) status = "MISSING_PARAMETER";
    char body[1024], sig[64]; unsigned char md[20]; unsigned int ml = 0;
    snprintf(body, sizeof body, "nonce=%s&otp=%s&status=%s&t=2026-11-03T12:00:00Z0000", nonce, otp, status);
    HMAC(EVP_sha1(), KEY, sizeof KEY, (unsigned char *)body, strlen(body), md, &ml);
    EVP_EncodeBlock((unsigned char *)sig, md, 20);
    if (!strcmp(mode[i], "forged")) sig[3] = sig[3] == 'A' ? 'B' : 'A';
    char resp[2048];
    int bl = snprintf(body, sizeof body, "h=%s\r\nt=2026-11-03T12:00:00Z0000\r\notp=%s\r\nnonce=%s\r\nstatus=%s\r\n\r\n", sig, otp, nonce, status);
    int rl = snprintf(resp, sizeof resp, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", bl, body);
    (void)!write(c, resp, rl); close(c);
  }
  _exit(0);
}

int main(void) {
  QT_BEGIN("yubico-c-client");
  signal(SIGALRM, on_alarm); alarm(90); atexit(stop);
  int ls = socket(AF_INET, SOCK_STREAM, 0), one = 1;
  setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  struct sockaddr_in a; memset(&a, 0, sizeof a); a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  socklen_t al = sizeof a;
  QT_CHECK(bind(ls, (struct sockaddr *)&a, sizeof a) == 0 && listen(ls, 8) == 0 && getsockname(ls, (struct sockaddr *)&a, &al) == 0,
           "validation server listening on 127.0.0.1");
  srv = fork();
  if (srv == 0) serve(ls);
  close(ls);
  char url[128]; snprintf(url, sizeof url, "http://127.0.0.1:%d/wsapi/2.0/verify", ntohs(a.sin_port));
  char b64key[64]; EVP_EncodeBlock((unsigned char *)b64key, KEY, sizeof KEY);

  QT_CHECK(ykclient_global_init() == YKCLIENT_OK, "ykclient_global_init (libcurl)");
  ykclient_t *ykc = NULL;
  QT_CHECK(ykclient_init(&ykc) == YKCLIENT_OK, "ykclient_init");
  QT_CHECK(ykclient_set_client_b64(ykc, 1, b64key) == YKCLIENT_OK, "ykclient_set_client_b64 (id 1, 20-byte key)");
  const char *bases[] = { url };
  QT_CHECK(ykclient_set_url_bases(ykc, 1, bases) == YKCLIENT_OK, "ykclient_set_url_bases (our server)");
  ykclient_set_verify_signature(ykc, 1);
  int rc = ykclient_request(ykc, OTP);
  QT_CHECK(rc == YKCLIENT_OK, "signed status=OK -> YKCLIENT_OK");
  if (rc != YKCLIENT_OK) printf("  info rc %d: %s\n", rc, ykclient_strerror(rc));
  const char *last = ykclient_get_last_url(ykc);
  QT_CHECK(strstr(last, "id=1") && strstr(last, "nonce=") && strstr(last, OTP) && strstr(last, "&h="),
           "the request named id, nonce, otp and carried a signature h=");
  rc = ykclient_request(ykc, OTP);
  QT_CHECK(rc == YKCLIENT_REPLAYED_OTP, "status=REPLAYED_OTP -> YKCLIENT_REPLAYED_OTP");
  rc = ykclient_request(ykc, OTP);
  QT_CHECK(rc == YKCLIENT_BAD_SERVER_SIGNATURE, "a forged response signature -> YKCLIENT_BAD_SERVER_SIGNATURE");
  rc = ykclient_request(ykc, OTP);
  QT_CHECK(rc == YKCLIENT_HMAC_ERROR, "a response for another OTP -> YKCLIENT_HMAC_ERROR");
  QT_CHECK(!strcmp(ykclient_strerror(YKCLIENT_BAD_SERVER_SIGNATURE), "Server response signature was invalid (BAD_SERVER_SIGNATURE)"),
           "ykclient_strerror text");
  ykclient_done(&ykc);

  /* the command-line tool, against the fifth (OK) answer */
  char cmd[512]; snprintf(cmd, sizeof cmd, "ykclient --url %s --apikey %s 1 %s > /tmp/ykclient.out 2>&1", url, b64key, OTP);
  int st = system(cmd);
  FILE *f = fopen("/tmp/ykclient.out", "r"); char line[256] = "";
  if (f) { if (!fgets(line, sizeof line, f)) line[0] = 0; fclose(f); }
  QT_CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 0, "ykclient tool: exit 0 (OTP verified)");
  if (!(WIFEXITED(st) && WEXITSTATUS(st) == 0)) printf("  info ykclient: %s", line);
  unlink("/tmp/ykclient.out");
  ykclient_global_done();
  QT_END();
}
