/* s2n-tls: a full TLS 1.3 handshake between an s2n server and an s2n client
 * over TCP loopback (two threads in this process), with an embedded
 * self-signed ECDSA P-256 certificate that the client verifies against its
 * trust store (host name "localhost"; a fixed wall clock inside the
 * certificate's validity, so the target's clock does not matter).  Then
 * application data both ways, and a client that does not trust the
 * certificate must fail the handshake. */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <s2n.h>

static const char CERT[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIBlTCCATugAwIBAgIUNk7lccCJL5MJ8fWJpxO1qQd3MPIwCgYIKoZIzj0EAwIw\n"
"FDESMBAGA1UEAwwJbG9jYWxob3N0MCAXDTI2MTAzMTEwMzYyNVoYDzIxMjYxMDA3\n"
"MTAzNjI1WjAUMRIwEAYDVQQDDAlsb2NhbGhvc3QwWTATBgcqhkjOPQIBBggqhkjO\n"
"PQMBBwNCAAS/mWzw5xPLdkTpsO5jy8YxjLRtcIDUbBrLsvyGWTj3ugUv0gx5jXyj\n"
"2i0BE9PjndW0A1BWEmu0yP1OKUYjfpYXo2kwZzAdBgNVHQ4EFgQUhyzALPVHitMP\n"
"CY22tdHBo39VULEwHwYDVR0jBBgwFoAUhyzALPVHitMPCY22tdHBo39VULEwDwYD\n"
"VR0TAQH/BAUwAwEB/zAUBgNVHREEDTALgglsb2NhbGhvc3QwCgYIKoZIzj0EAwID\n"
"SAAwRQIhAIvBy0ZqxS43/YTEYY/KICkovyzqdEwqduntuTwXbaL3AiBPsBvahdnS\n"
"1UJi7N4UzDIqi4fb764x0hhRRkZlpUDblA==\n"
"-----END CERTIFICATE-----\n";
static const char KEY[] =
"-----BEGIN PRIVATE KEY-----\n"
"MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQg8kXE/vYKRHzN3PDY\n"
"mNNAhGtcDC2pubdOGmQZ26HW8kyhRANCAAS/mWzw5xPLdkTpsO5jy8YxjLRtcIDU\n"
"bBrLsvyGWTj3ugUv0gx5jXyj2i0BE9PjndW0A1BWEmu0yP1OKUYjfpYX\n"
"-----END PRIVATE KEY-----\n";

/* notBefore of CERT (2026-10-31 10:36:25 UTC) + one day */
static int fixed_clock(void *ctx, uint64_t *ns) { (void)ctx; *ns = (1793442985ULL + 86400ULL) * 1000000000ULL; return 0; }
static void err(const char *what) {
  const char *m = s2n_strerror(s2n_errno, "EN"), *d = s2n_strerror_debug(s2n_errno, "EN");
  printf("  info %s: %s [%s]\n", what, m ? m : "?", d ? d : "?");
}

static int lfd = -1;
static struct s2n_config *scfg;
static int srv_ok = 0, srv_hs = 0;
static char srv_got[64];
static void *server(void *arg) {
  int rounds = *(int *)arg;
  for (int r = 0; r < rounds; r++) {
    int c = accept(lfd, NULL, NULL);
    if (c < 0) return NULL;
    struct s2n_connection *conn = s2n_connection_new(S2N_SERVER);
    s2n_connection_set_config(conn, scfg);
    s2n_connection_set_fd(conn, c);
    s2n_blocked_status bl;
    if (s2n_negotiate(conn, &bl) == S2N_SUCCESS) {
      if (r == 0) srv_hs = 1;
      ssize_t n = s2n_recv(conn, srv_got, sizeof srv_got - 1, &bl);
      if (n > 0) {
        srv_got[n] = 0;
        char rep[80]; snprintf(rep, sizeof rep, "ACK:%s", srv_got);
        if (s2n_send(conn, rep, (ssize_t)strlen(rep), &bl) == (ssize_t)strlen(rep)) srv_ok = 1;
      }
      s2n_shutdown(conn, &bl);
    }
    s2n_connection_free(conn);
    close(c);
  }
  return NULL;
}
static int dial(unsigned short port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in a; memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(fd, (struct sockaddr *)&a, sizeof a) != 0) { close(fd); return -1; }
  return fd;
}

int main(void) {
  QT_BEGIN("s2n-tls");
  setenv("S2N_DONT_MLOCK", "1", 1);
  int rc = s2n_init();
  if (rc != S2N_SUCCESS) err("s2n_init");
  QT_CHECK(rc == S2N_SUCCESS, "s2n_init");
  printf("  info libcrypto version 0x%lx\n", s2n_get_openssl_version());

  struct s2n_cert_chain_and_key *ck = s2n_cert_chain_and_key_new();
  rc = s2n_cert_chain_and_key_load_pem(ck, CERT, KEY);
  if (rc) err("load_pem");
  QT_CHECK(rc == S2N_SUCCESS, "certificate chain and ECDSA key loaded from PEM");
  scfg = s2n_config_new();
  QT_CHECK(s2n_config_set_cipher_preferences(scfg, "default_tls13") == S2N_SUCCESS, "server policy default_tls13");
  QT_CHECK(s2n_config_add_cert_chain_and_key_to_store(scfg, ck) == S2N_SUCCESS, "server certificate installed");
  s2n_config_set_wall_clock(scfg, fixed_clock, NULL);

  struct s2n_config *ccfg = s2n_config_new();
  s2n_config_set_cipher_preferences(ccfg, "default_tls13");
  s2n_config_set_wall_clock(ccfg, fixed_clock, NULL);
  rc = s2n_config_add_pem_to_trust_store(ccfg, CERT);
  if (rc) err("trust store");
  QT_CHECK(rc == S2N_SUCCESS, "client trusts the self-signed certificate");
  struct s2n_config *bad = s2n_config_new();   /* no trust anchors besides the (empty) defaults */
  s2n_config_wipe_trust_store(bad);
  s2n_config_set_cipher_preferences(bad, "default_tls13");
  s2n_config_set_wall_clock(bad, fixed_clock, NULL);

  lfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sa; memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET; sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  QT_CHECK(bind(lfd, (struct sockaddr *)&sa, sizeof sa) == 0 && listen(lfd, 4) == 0, "listen on 127.0.0.1");
  socklen_t sl = sizeof sa; getsockname(lfd, (struct sockaddr *)&sa, &sl);
  unsigned short port = ntohs(sa.sin_port);
  int rounds = 2;
  pthread_t th; pthread_create(&th, NULL, server, &rounds);

  /* round 1: trusted handshake + data */
  int fd = dial(port);
  QT_CHECK(fd >= 0, "TCP connect");
  struct s2n_connection *c = s2n_connection_new(S2N_CLIENT);
  s2n_connection_set_config(c, ccfg);
  s2n_set_server_name(c, "localhost");
  s2n_connection_set_fd(c, fd);
  s2n_blocked_status bl;
  rc = s2n_negotiate(c, &bl);
  if (rc) err("client negotiate");
  QT_CHECK(rc == S2N_SUCCESS, "TLS handshake completes, peer certificate verified");
  int ver = s2n_connection_get_actual_protocol_version(c);
  const char *ciph = s2n_connection_get_cipher(c);
  printf("  info protocol %d, cipher %s\n", ver, ciph ? ciph : "(null)");
  QT_CHECK(ver == S2N_TLS13, "negotiated TLS 1.3");
  QT_CHECK(ciph && (strstr(ciph, "AES") || strstr(ciph, "CHACHA")), "an AEAD cipher suite");
  const char *msg = "telemetry frame 42";
  QT_CHECK(s2n_send(c, msg, (ssize_t)strlen(msg), &bl) == (ssize_t)strlen(msg), "client s2n_send");
  char got[96] = { 0 }; ssize_t n, tot = 0;
  while (tot < (ssize_t)sizeof got - 1 && (n = s2n_recv(c, got + tot, sizeof got - 1 - tot, &bl)) > 0) tot += n;
  printf("  info client received \"%s\"\n", got);
  QT_CHECK(strcmp(got, "ACK:telemetry frame 42") == 0, "server echo received over TLS");
  s2n_shutdown(c, &bl);
  s2n_connection_free(c); close(fd);

  /* round 2: a client with no trust anchor must reject the server */
  fd = dial(port);
  c = s2n_connection_new(S2N_CLIENT);
  s2n_connection_set_config(c, bad);
  s2n_set_server_name(c, "localhost");
  s2n_connection_set_fd(c, fd);
  rc = s2n_negotiate(c, &bl);
  int et = s2n_error_get_type(s2n_errno);
  if (rc) err("untrusted negotiate (expected)");
  QT_CHECK(rc != S2N_SUCCESS, "untrusted certificate is rejected");
  printf("  info error type %d\n", et);
  s2n_connection_free(c); close(fd);   /* the server sees an alert / EOF and returns */

  pthread_join(th, NULL);
  QT_CHECK(srv_hs && srv_ok && strcmp(srv_got, "telemetry frame 42") == 0, "server decrypted the client's data");
  close(lfd);
  s2n_config_free(bad); s2n_config_free(ccfg); s2n_config_free(scfg);
  s2n_cert_chain_and_key_free(ck);
  QT_CHECK(s2n_cleanup() == S2N_SUCCESS, "s2n_cleanup");
  QT_END();
}
