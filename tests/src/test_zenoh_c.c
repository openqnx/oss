/* zenoh-c: publish/subscribe between two sessions over a TCP link.
 *
 * One session listens on loopback TCP, the other connects to it and
 * subscribes; the first publishes until the sample arrives. That exercises
 * the Rust runtime (threads, timers), the TCP transport and the socket code
 * the QNX crate patches touch, not just an in-process shortcut. Multicast
 * scouting is off: the sessions find each other by the configured endpoint. */
#include "common.h"
#include <unistd.h>
#include <zenoh.h>

static volatile int received;
static char text[64];

static void on_sample(z_loaned_sample_t *sample, void *ctx) {
  (void)ctx;
  z_owned_string_t s;
  if (z_bytes_to_string(z_sample_payload(sample), &s) == Z_OK) {
    snprintf(text, sizeof text, "%.*s", (int)z_string_len(z_loan(s)), z_string_data(z_loan(s)));
    z_drop(z_move(s));
  }
  received++;
}

/* A peer also listens by default, on tcp/[::]:0 -- which fails where the
 * stack has no IPv6 (a default mkqnximage image). Give every session an IPv4
 * listener instead, and the endpoint to reach the other one by. */
static int open_session(z_owned_session_t *s, const char *listen, const char *connect) {
  z_owned_config_t cfg;
  if (z_config_default(&cfg) != Z_OK) return 0;
  if (zc_config_insert_json5(z_loan_mut(cfg), "scouting/multicast/enabled", "false") != Z_OK) return 0;
  if (zc_config_insert_json5(z_loan_mut(cfg), "listen/endpoints", listen) != Z_OK) return 0;
  if (connect && zc_config_insert_json5(z_loan_mut(cfg), "connect/endpoints", connect) != Z_OK) return 0;
  return z_open(s, z_move(cfg), NULL) == Z_OK;
}

int main(void) {
  QT_BEGIN("zenoh-c");
  zc_init_log_from_env_or("error");   /* zenoh's own errors go to stderr; RUST_LOG for more */
  z_owned_session_t pub, sub;
  int up = open_session(&pub, "[\"tcp/127.0.0.1:17447\"]", NULL);
  QT_CHECK(up, "open a session listening on TCP");
  if (!up) QT_END();
  up = open_session(&sub, "[\"tcp/127.0.0.1:0\"]", "[\"tcp/127.0.0.1:17447\"]");
  QT_CHECK(up, "open a second session connecting to it");
  if (!up) { z_drop(z_move(pub)); QT_END(); }

  z_view_keyexpr_t ke;
  QT_CHECK(z_view_keyexpr_from_str(&ke, "qnx/port-test") == Z_OK, "key expression");
  z_owned_closure_sample_t cb;
  z_closure_sample(&cb, on_sample, NULL, NULL);
  z_owned_subscriber_t s;
  QT_CHECK(z_declare_subscriber(z_loan(sub), &s, z_loan(ke), z_move(cb), NULL) == Z_OK, "declare a subscriber");

  /* The subscription reaches the publishing side asynchronously: publish
   * until it arrives, for up to 10 s. */
  for (int i = 0; i < 100 && !received; i++) {
    z_owned_bytes_t payload;
    z_bytes_copy_from_str(&payload, "hello from qnx");
    z_put(z_loan(pub), z_loan(ke), z_move(payload), NULL);
    usleep(100000);
  }
  QT_CHECK(received > 0, "the subscriber received a sample over TCP");
  QT_CHECK(!strcmp(text, "hello from qnx"), "with the payload intact");

  z_drop(z_move(s));
  z_drop(z_move(sub));
  z_drop(z_move(pub));
  QT_END();
}
