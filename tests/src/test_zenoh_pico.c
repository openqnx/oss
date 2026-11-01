/* zenoh-pico: publish/subscribe between two peer sessions in one process.
 *
 * zenoh-pico 1.2 peers talk over UDP multicast (unicast peer listening is
 * not implemented in this release; unicast is client-to-router), so both
 * sessions join one multicast group on the loopback interface and the second
 * publishes until the first one's subscriber has the sample. That runs the
 * read and lease threads, the UDP multicast link and the unix system layer
 * the QNX patches add (random session ids from /dev/urandom). Also: the two
 * sessions must have different ids, and a serialized uint32 round trip. */
#include "common.h"
#include <unistd.h>
#include <zenoh-pico.h>

#define LOCATOR "udp/224.0.0.123:17448#iface=lo0"
#define KEY "qnx/port-test/pico"

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

static int open_peer(z_owned_session_t *s) {
  z_owned_config_t cfg;
  z_config_default(&cfg);
  zp_config_insert(z_loan_mut(cfg), Z_CONFIG_MODE_KEY, "peer");
  zp_config_insert(z_loan_mut(cfg), Z_CONFIG_LISTEN_KEY, LOCATOR);
  if (z_open(s, z_move(cfg), NULL) != Z_OK) return 0;
  if (zp_start_read_task(z_loan_mut(*s), NULL) != Z_OK || zp_start_lease_task(z_loan_mut(*s), NULL) != Z_OK) {
    z_drop(z_move(*s));
    return 0;
  }
  return 1;
}

int main(void) {
  QT_BEGIN("zenoh-pico");

  z_owned_bytes_t b;
  uint32_t v = 0;
  QT_CHECK(ze_serialize_uint32(&b, 0xC0FFEEu) == Z_OK && ze_deserialize_uint32(z_loan(b), &v) == Z_OK
           && v == 0xC0FFEEu, "serialize/deserialize uint32 0xC0FFEE");
  QT_CHECK(z_bytes_len(z_loan(b)) == 4, "a uint32 serializes to 4 bytes");
  z_drop(z_move(b));

  z_owned_session_t sub, pub;
  int up = open_peer(&sub);
  QT_CHECK(up, "open a peer session on " LOCATOR);
  if (!up) QT_END();
  up = open_peer(&pub);
  QT_CHECK(up, "open a second peer session on the same group");
  if (!up) { z_drop(z_move(sub)); QT_END(); }

  z_id_t a = z_info_zid(z_loan(sub)), c = z_info_zid(z_loan(pub));
  QT_CHECK(memcmp(a.id, c.id, sizeof a.id) != 0, "the two sessions have different random ids");

  z_view_keyexpr_t ke;
  QT_CHECK(z_view_keyexpr_from_str(&ke, KEY) == Z_OK, "key expression " KEY);
  z_owned_closure_sample_t cb;
  z_closure(&cb, on_sample, NULL, NULL);
  z_owned_subscriber_t s;
  QT_CHECK(z_declare_subscriber(z_loan(sub), &s, z_loan(ke), z_move(cb), NULL) == Z_OK, "declare a subscriber");

  /* Publish until the sample arrives, for up to 10 s. */
  for (int i = 0; i < 100 && !received; i++) {
    z_owned_bytes_t payload;
    z_bytes_copy_from_str(&payload, "hello from zenoh-pico");
    z_put(z_loan(pub), z_loan(ke), z_move(payload), NULL);
    usleep(100000);
  }
  QT_CHECK(received > 0, "the subscriber received a sample over UDP multicast");
  QT_CHECK(!strcmp(text, "hello from zenoh-pico"), "with the payload intact");

  z_drop(z_move(s));
  z_drop(z_move(pub));
  z_drop(z_move(sub));
  QT_END();
}
