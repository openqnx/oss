/* glib-networking: the GnuTLS TLS backend for GIO. A TLS server and client
 * (two threads, 127.0.0.1) complete a handshake with an embedded self-signed
 * test certificate and exchange data; the client first sees the certificate
 * rejected as an unknown CA (accept-certificate), then, with the certificate
 * as its trust anchor, validates it cleanly. */
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gio/gio.h>

/* Throwaway test-only key pair: CN=qnx-port-test, SAN localhost/127.0.0.1,
   valid 2000-01-01 .. 2099-12-31. */
static const char *CERT =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIBQDCB5qADAgECAgIQkjAKBggqhkjOPQQDAjAYMRYwFAYDVQQDDA1xbngtcG9y\n"
  "dC10ZXN0MCAXDTAwMDEwMTAwMDAwMFoYDzIwOTkxMjMxMDAwMDAwWjAYMRYwFAYD\n"
  "VQQDDA1xbngtcG9ydC10ZXN0MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEa2wf\n"
  "StrAv7a0MwjawxfqYsiFhFen58myh+/C1gY/18ddabkJbDXK6YBPUoJiARcRapR2\n"
  "LrdkFE5okBDZHhVy1aMeMBwwGgYDVR0RBBMwEYIJbG9jYWxob3N0hwR/AAABMAoG\n"
  "CCqGSM49BAMCA0kAMEYCIQCdLQJshbwLou9XeNIsGC/MQIjfXCBQ7KOcOJWfszQI\n"
  "DQIhAIRlkZodoq0pmzvOR/ri432ZLwy9xl8LzyunErUv90xm\n"
  "-----END CERTIFICATE-----\n";
static const char *KEY =
  "-----BEGIN PRIVATE KEY-----\n"
  "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgoBCeJVIBakvHvZln\n"
  "rFJnWS0S19iLNVryIMmNPvmjjxKhRANCAARrbB9K2sC/trQzCNrDF+piyIWEV6fn\n"
  "ybKH78LWBj/Xx11puQlsNcrpgE9SgmIBFxFqlHYut2QUTmiQENkeFXLV\n"
  "-----END PRIVATE KEY-----\n";

static GTlsCertificate *cert;
static GSocketListener *listener;
static char srv_got[8];
static gpointer server(gpointer ud) {
  (void)ud;
  GError *e = NULL;
  GSocketConnection *c = g_socket_listener_accept(listener, NULL, NULL, &e);
  if (!c) { printf("  info accept: %s\n", e->message); g_error_free(e); return NULL; }
  GIOStream *t = g_tls_server_connection_new(G_IO_STREAM(c), cert, &e);
  if (t && g_tls_connection_handshake(G_TLS_CONNECTION(t), NULL, &e)) {
    gsize n = 0;
    memset(srv_got, 0, sizeof srv_got);
    g_input_stream_read_all(g_io_stream_get_input_stream(t), srv_got, 4, &n, NULL, &e);
    g_output_stream_write_all(g_io_stream_get_output_stream(t), "pong", 4, &n, NULL, NULL);
    g_io_stream_close(t, NULL, NULL);
  } else if (e) { printf("  info server: %s\n", e->message); }
  if (e) g_error_free(e);
  if (t) g_object_unref(t);
  g_object_unref(c);
  return NULL;
}
static GTlsCertificateFlags seen_errors;
static int asked;
static gboolean accept_cert(GTlsConnection *conn, GTlsCertificate *peer, GTlsCertificateFlags errors, gpointer ud) {
  (void)conn; (void)peer; (void)ud;
  asked = 1; seen_errors = errors;
  return TRUE;
}
struct result { int handshake, pong, same_cert; char proto[32], suite[96]; };
static struct result session(guint16 port, GTlsDatabase *db) {
  struct result r; memset(&r, 0, sizeof r);
  GError *e = NULL;
  memset(srv_got, 0, sizeof srv_got);
  GThread *th = g_thread_new("tls-server", server, NULL);
  GSocketClient *sc = g_socket_client_new();
  g_socket_client_set_enable_proxy(sc, FALSE);   /* loopback: no proxy resolver */
  GSocketConnection *c = g_socket_client_connect_to_host(sc, "127.0.0.1", port, NULL, &e);
  GSocketConnectable *id = g_network_address_new("localhost", port);
  GIOStream *t = c ? g_tls_client_connection_new(G_IO_STREAM(c), id, &e) : NULL;
  if (t) {
    if (db) g_tls_connection_set_database(G_TLS_CONNECTION(t), db);
    g_signal_connect(t, "accept-certificate", G_CALLBACK(accept_cert), NULL);
    r.handshake = g_tls_connection_handshake(G_TLS_CONNECTION(t), NULL, &e);
    if (r.handshake) {
      gsize n = 0; char buf[8] = {0};
      g_output_stream_write_all(g_io_stream_get_output_stream(t), "ping", 4, &n, NULL, NULL);
      g_input_stream_read_all(g_io_stream_get_input_stream(t), buf, 4, &n, NULL, NULL);
      r.pong = n == 4 && memcmp(buf, "pong", 4) == 0;
      GTlsCertificate *peer = g_tls_connection_get_peer_certificate(G_TLS_CONNECTION(t));
      r.same_cert = peer && g_tls_certificate_is_same(peer, cert);
      GTlsProtocolVersion v = g_tls_connection_get_protocol_version(G_TLS_CONNECTION(t));
      snprintf(r.proto, sizeof r.proto, "%s", v == G_TLS_PROTOCOL_VERSION_TLS_1_3 ? "TLS 1.3" :
               v == G_TLS_PROTOCOL_VERSION_TLS_1_2 ? "TLS 1.2" : "other");
      gchar *cs = g_tls_connection_get_ciphersuite_name(G_TLS_CONNECTION(t));
      snprintf(r.suite, sizeof r.suite, "%s", cs ? cs : "(none)");
      g_free(cs);
      g_io_stream_close(t, NULL, NULL);
    }
  }
  if (e) { printf("  info client: %s\n", e->message); g_error_free(e); }
  if (t) g_object_unref(t);
  if (c) { g_io_stream_close(G_IO_STREAM(c), NULL, NULL); g_object_unref(c); }
  g_object_unref(id); g_object_unref(sc);
  g_thread_join(th);
  return r;
}

int main(void) {
  QT_BEGIN("glib-networking");
  alarm(60);
  /* No GIO_EXTRA_MODULES: libgio must find the module in its own directory. */
  GTlsBackend *be = g_tls_backend_get_default();
  printf("  info TLS backend %s\n", be ? G_OBJECT_TYPE_NAME(be) : "(none)");
  QT_CHECK(be && g_tls_backend_supports_tls(be), "a TLS-capable backend is loaded");
  QT_CHECK(be && strstr(G_OBJECT_TYPE_NAME(be), "Gnutls") != NULL, "it is glib-networking's GnuTLS backend");
  if (!be || !g_tls_backend_supports_tls(be)) QT_END();

  GError *e = NULL;
  char pem[2048];
  snprintf(pem, sizeof pem, "%s%s", CERT, KEY);
  cert = g_tls_certificate_new_from_pem(pem, -1, &e);
  QT_CHECK(cert != NULL, "certificate + key parsed from PEM");
  if (!cert) { printf("  info %s\n", e->message); QT_END(); }
  gchar *subj = NULL;
  g_object_get(cert, "subject-name", &subj, NULL);
  printf("  info subject %s\n", subj ? subj : "(null)");
  QT_CHECK(subj && strstr(subj, "qnx-port-test") != NULL, "certificate subject name");
  g_free(subj);

  listener = g_socket_listener_new();
  GInetAddress *lo = g_inet_address_new_loopback(G_SOCKET_FAMILY_IPV4);
  GSocketAddress *sa = g_inet_socket_address_new(lo, 0), *eff = NULL;
  QT_CHECK(g_socket_listener_add_address(listener, sa, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, NULL, &eff, &e),
           "listening on 127.0.0.1");
  guint16 port = eff ? g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(eff)) : 0;

  asked = 0;
  struct result r = session(port, NULL);
  printf("  info %s, %s; certificate errors 0x%x\n", r.proto, r.suite, (unsigned)seen_errors);
  QT_CHECK(r.handshake, "TLS handshake completes");
  QT_CHECK(asked && (seen_errors & G_TLS_CERTIFICATE_UNKNOWN_CA), "self-signed certificate flagged as unknown CA");
  QT_CHECK(strcmp(srv_got, "ping") == 0 && r.pong, "ping/pong over TLS");
  QT_CHECK(r.same_cert, "client sees the server's certificate");
  QT_CHECK(strcmp(r.proto, "other") != 0, "TLS 1.2 or 1.3 negotiated");

  const char *ca = "/tmp/qnxport-glibnet-ca.pem";
  FILE *f = fopen(ca, "w");
  if (f) { fputs(CERT, f); fclose(f); }
  GTlsDatabase *db = g_tls_file_database_new(ca, &e);
  QT_CHECK(db != NULL, "file database with the certificate as anchor");
  if (db) {
    asked = 0; seen_errors = 0;
    r = session(port, db);
    QT_CHECK(r.handshake && r.pong, "second session with the trust anchor");
    QT_CHECK(!asked || (seen_errors & ~(G_TLS_CERTIFICATE_NOT_ACTIVATED | G_TLS_CERTIFICATE_EXPIRED)) == 0,
             "certificate validates for localhost against the anchor");
    if (asked) printf("  info validation still reported 0x%x (clock?)\n", (unsigned)seen_errors);
    g_object_unref(db);
  }
  unlink(ca);
  g_socket_listener_close(listener);
  g_object_unref(listener); g_object_unref(sa); if (eff) g_object_unref(eff); g_object_unref(lo);
  g_object_unref(cert);
  QT_END();
}
