/* rust: the port is the Rust 1.90 toolchain with a standard library for
 * x86_64-pc-nto-qnx800 / aarch64-unknown-nto-qnx800. It runs on the build
 * host; what reaches a QNX target is Rust std inside Rust-built code. This
 * test links such code statically -- zenoh-c's libzenohc.a, built with that
 * toolchain -- so the program carries its own copy of Rust std for QNX, and
 * exercises it: pure Rust logic (key-expression algebra, serialisation),
 * the OS random source and clocks, and a session whose runtime threads,
 * timers and loopback TCP listener answer a query sent to itself. The
 * package's note about the toolchain must be installed. */
#include "common.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <zenoh.h>

static volatile int queries, replies;
static char reply_text[64];

static void on_query(z_loaned_query_t *q, void *ctx) {
  (void)ctx;
  z_view_string_t params; z_query_parameters(q, &params);
  char pbuf[64]; int n = 0;                 /* a view: not NUL-terminated */
  snprintf(pbuf, sizeof pbuf, "%.*s", (int)z_string_len(z_loan(params)), z_string_data(z_loan(params)));
  sscanf(pbuf, "n=%d", &n);
  uint64_t sum = 0;
  for (int i = 1; i <= n; i++) sum += (uint64_t)i;
  z_owned_bytes_t payload; ze_serialize_uint64(&payload, sum);
  z_query_reply(q, z_query_keyexpr(q), z_move(payload), NULL);
  queries++;
}
static void on_reply(z_loaned_reply_t *r, void *ctx) {
  (void)ctx;
  if (!z_reply_is_ok(r)) return;
  uint64_t v = 0;
  if (ze_deserialize_uint64(z_sample_payload(z_reply_ok(r)), &v) == Z_OK) snprintf(reply_text, sizeof reply_text, "%llu", (unsigned long long)v);
  replies++;
}

static int ke(z_view_keyexpr_t *k, const char *s) { return z_view_keyexpr_from_str(k, s) == Z_OK; }

int main(void) {
  QT_BEGIN("rust");
  const char *notes[] = { "/usr/local/share/doc/rust-qnx/README", "/data/oss/usr/local/share/doc/rust-qnx/README" };
  char line[256] = ""; FILE *f = NULL;
  for (int i = 0; i < 2 && !f; i++) f = fopen(notes[i], "r");
  if (f) { if (!fgets(line, sizeof line, f)) line[0] = 0; fclose(f); }
  printf("  info note: %s", line[0] ? line : "(none)\n");
  QT_CHECK(strstr(line, "Rust 1.90") && strstr(line, "x86_64-pc-nto-qnx800") && strstr(line, "aarch64-unknown-nto-qnx800"),
           "share/doc/rust-qnx/README: Rust 1.90, both QNX targets");

  /* pure Rust: key expression algebra */
  z_view_keyexpr_t a, b, c, d;
  QT_CHECK(ke(&a, "robot/*/sensor/**") && ke(&b, "robot/arm/sensor/temp/0") && ke(&c, "robot/arm/motor") && ke(&d, "robot/**"),
           "key expressions parsed");
  QT_CHECK(z_keyexpr_includes(z_loan(a), z_loan(b)) && !z_keyexpr_includes(z_loan(a), z_loan(c)), "robot/*/sensor/** includes .../temp/0, not robot/arm/motor");
  QT_CHECK(z_keyexpr_intersects(z_loan(d), z_loan(c)) && z_keyexpr_includes(z_loan(d), z_loan(a)), "robot/** intersects and includes");
  char raw[] = "a/**/**/b";
  QT_CHECK(z_keyexpr_canonize_null_terminated(raw) == Z_OK && !strcmp(raw, "a/**/b"), "canonize a/**/**/b -> a/**/b");
  z_view_keyexpr_t bad;
  QT_CHECK(z_view_keyexpr_from_str(&bad, "a//b") != Z_OK, "a//b rejected");
  z_owned_keyexpr_t j; z_view_keyexpr_t l, r;
  ke(&l, "fleet/7"); ke(&r, "odom");
  QT_CHECK(z_keyexpr_join(&j, z_loan(l), z_loan(r)) == Z_OK, "join fleet/7 + odom");
  z_view_string_t js; z_keyexpr_as_view_string(z_loan(j), &js);
  QT_CHECK(z_string_len(z_loan(js)) == 12 && !strncmp(z_string_data(z_loan(js)), "fleet/7/odom", 12), "-> fleet/7/odom");
  z_drop(z_move(j));

  /* serialisation */
  z_owned_bytes_t bytes; double dv = 0; uint64_t uv = 0; z_owned_string_t sv;
  ze_serialize_double(&bytes, 3.141592653589793);
  QT_CHECK(ze_deserialize_double(z_loan(bytes), &dv) == Z_OK && dv == 3.141592653589793, "double round trip");
  z_drop(z_move(bytes));
  ze_serialize_uint64(&bytes, 0xFEEDFACECAFEBEEFull);
  QT_CHECK(ze_deserialize_uint64(z_loan(bytes), &uv) == Z_OK && uv == 0xFEEDFACECAFEBEEFull && z_bytes_len(z_loan(bytes)) == 8, "uint64 round trip, 8 bytes");
  z_drop(z_move(bytes));
  ze_serialize_str(&bytes, "grüß QNX");
  QT_CHECK(ze_deserialize_string(z_loan(bytes), &sv) == Z_OK && z_string_len(z_loan(sv)) == strlen("grüß QNX")
           && !strncmp(z_string_data(z_loan(sv)), "grüß QNX", z_string_len(z_loan(sv))), "UTF-8 string round trip");
  z_drop(z_move(sv)); z_drop(z_move(bytes));

  /* std: OS randomness and clocks */
  uint32_t r1 = z_random_u32(), r2 = z_random_u32(), r3 = z_random_u32();
  QT_CHECK(!(r1 == r2 && r2 == r3), "z_random_u32: OS random source");
  z_clock_t t0 = z_clock_now();
  z_sleep_ms(30);
  uint64_t el = z_clock_elapsed_ms(&t0);
  printf("  info slept 30 ms, monotonic clock says %llu ms\n", (unsigned long long)el);
  QT_CHECK(el >= 25 && el < 5000, "z_sleep_ms / monotonic clock");

  /* a session: runtime threads, a TCP listener on loopback, a query to itself */
  zc_init_log_from_env_or("error");
  z_owned_config_t cfg; z_config_default(&cfg);
  zc_config_insert_json5(z_loan_mut(cfg), "scouting/multicast/enabled", "false");
  zc_config_insert_json5(z_loan_mut(cfg), "listen/endpoints", "[\"tcp/127.0.0.1:0\"]");
  z_owned_session_t s;
  int up = z_open(&s, z_move(cfg), NULL) == Z_OK;
  QT_CHECK(up, "session opened (peer, TCP listener on 127.0.0.1)");
  if (up) {
    z_id_t zid = z_info_zid(z_loan(s));
    z_owned_string_t zs; z_id_to_string(&zid, &zs);
    printf("  info zid %.*s\n", (int)z_string_len(z_loan(zs)), z_string_data(z_loan(zs)));
    QT_CHECK(z_string_len(z_loan(zs)) >= 16, "session id");
    z_drop(z_move(zs));
    z_timestamp_t ts;
    /* uhlc's NTP64: seconds since the Unix epoch in the upper 32 bits */
    int tok = z_timestamp_new(&ts, z_loan(s)) == Z_OK;
    long long tsec = tok ? (long long)(z_timestamp_ntp64_time(&ts) >> 32) : 0, now = (long long)time(NULL);
    printf("  info timestamp %lld s, time() %lld s\n", tsec, now);
    QT_CHECK(tok && tsec - now < 5 && now - tsec < 5, "session timestamp agrees with the system clock (NTP64, Unix epoch)");
    z_view_keyexpr_t qk; ke(&qk, "qnx/rust/sum");
    z_owned_closure_query_t qc; z_closure_query(&qc, on_query, NULL, NULL);
    z_owned_queryable_t qa;
    QT_CHECK(z_declare_queryable(z_loan(s), &qa, z_loan(qk), z_move(qc), NULL) == Z_OK, "queryable declared");
    z_owned_closure_reply_t rc; z_closure_reply(&rc, on_reply, NULL, NULL);
    QT_CHECK(z_get(z_loan(s), z_loan(qk), "n=100", z_move(rc), NULL) == Z_OK, "query sent");
    for (int i = 0; i < 100 && !replies; i++) usleep(50000);
    printf("  info %d query(ies), %d reply(ies): %s\n", queries, replies, reply_text);
    QT_CHECK(queries == 1 && replies == 1 && !strcmp(reply_text, "5050"), "queryable answered on a runtime thread: sum 1..100 = 5050");
    z_drop(z_move(qa));
    z_drop(z_move(s));
  }
  QT_END();
}
