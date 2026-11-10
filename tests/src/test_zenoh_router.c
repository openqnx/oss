/* zenoh-router: zenohd routes between two zenoh-c clients.
 *
 * The test starts zenohd listening on loopback TCP (multicast scouting off),
 * then opens two sessions in *client* mode connected to it. Clients never
 * talk to each other directly, so a sample published by one reaching the
 * other's subscriber was routed by zenohd. A query on the router's admin
 * space (@/<zid>/router) must be answered by zenohd itself with its JSON
 * description, naming the endpoint it listens on. After zenohd is stopped,
 * nothing more arrives. */
#include "common.h"
#include <signal.h>
#include <spawn.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <zenoh.h>

extern char **environ;
static volatile int received, replies, router_ok;
static char text[64], rkey[128];

static void on_sample(z_loaned_sample_t *sample, void *ctx) {
  (void)ctx;
  z_owned_string_t s;
  if (z_bytes_to_string(z_sample_payload(sample), &s) == Z_OK) {
    snprintf(text, sizeof text, "%.*s", (int)z_string_len(z_loan(s)), z_string_data(z_loan(s)));
    z_drop(z_move(s));
  }
  received++;
}

static void on_reply(z_loaned_reply_t *reply, void *ctx) {
  (void)ctx;
  if (!z_reply_is_ok(reply)) return;
  const z_loaned_sample_t *smp = z_reply_ok(reply);
  z_view_string_t k;
  z_keyexpr_as_view_string(z_sample_keyexpr(smp), &k);
  snprintf(rkey, sizeof rkey, "%.*s", (int)z_string_len(z_loan(k)), z_string_data(z_loan(k)));
  z_owned_string_t s;
  if (z_bytes_to_string(z_sample_payload(smp), &s) == Z_OK) {
    size_t n = z_string_len(z_loan(s));
    char *j = malloc(n + 1);
    memcpy(j, z_string_data(z_loan(s)), n); j[n] = 0;
    if (strstr(j, "tcp/127.0.0.1:17457") && strstr(j, "\"zid\"")) router_ok = 1;
    free(j);
    z_drop(z_move(s));
  }
  replies++;
}

static int open_client(z_owned_session_t *s) {
  z_owned_config_t cfg;
  if (z_config_default(&cfg) != Z_OK) return 0;
  if (zc_config_insert_json5(z_loan_mut(cfg), "mode", "\"client\"") != Z_OK) return 0;
  if (zc_config_insert_json5(z_loan_mut(cfg), "scouting/multicast/enabled", "false") != Z_OK) return 0;
  if (zc_config_insert_json5(z_loan_mut(cfg), "connect/endpoints", "[\"tcp/127.0.0.1:17457\"]") != Z_OK) return 0;
  return z_open(s, z_move(cfg), NULL) == Z_OK;
}

int main(void) {
  QT_BEGIN("zenoh-router");
  zc_init_log_from_env_or("error");
  setenv("RUST_LOG", "warn", 0);   /* zenohd prints its whole configuration at "info" */
  const char *zenohd = access("/usr/local/bin/zenohd", X_OK) == 0 ? "/usr/local/bin/zenohd" : "zenohd";
  char *argv[] = {(char *)"zenohd", (char *)"--no-multicast-scouting", (char *)"-l", (char *)"tcp/127.0.0.1:17457", NULL};
  pid_t pid;
  int rc = posix_spawnp(&pid, zenohd, NULL, NULL, argv, environ);
  QT_CHECK(rc == 0, "start zenohd listening on tcp/127.0.0.1:17457");
  if (rc != 0) QT_END();

  z_owned_session_t a, b;
  int up = 0;
  for (int i = 0; i < 50 && !up; i++) { usleep(200000); up = open_client(&a); }  /* zenohd needs a moment */
  QT_CHECK(up, "client session A connects to the router");
  if (!up) { kill(pid, SIGTERM); waitpid(pid, NULL, 0); QT_END(); }
  up = open_client(&b);
  QT_CHECK(up, "client session B connects to the router");
  if (!up) { z_drop(z_move(a)); kill(pid, SIGTERM); waitpid(pid, NULL, 0); QT_END(); }

  z_view_keyexpr_t ke, sub_ke, adm;
  z_view_keyexpr_from_str(&ke, "qnx/router-test/data");
  z_view_keyexpr_from_str(&sub_ke, "qnx/router-test/**");
  z_owned_closure_sample_t cb;
  z_closure_sample(&cb, on_sample, NULL, NULL);
  z_owned_subscriber_t s;
  QT_CHECK(z_declare_subscriber(z_loan(b), &s, z_loan(sub_ke), z_move(cb), NULL) == Z_OK, "B subscribes to qnx/router-test/**");
  for (int i = 0; i < 100 && !received; i++) {
    z_owned_bytes_t payload;
    z_bytes_copy_from_str(&payload, "routed by zenohd");
    z_put(z_loan(a), z_loan(ke), z_move(payload), NULL);
    usleep(100000);
  }
  QT_CHECK(received > 0 && !strcmp(text, "routed by zenohd"), "A's sample reached B through the router, payload intact");

  z_view_keyexpr_from_str(&adm, "@/*/router");
  z_owned_closure_reply_t rcb;
  z_closure_reply(&rcb, on_reply, NULL, NULL);
  QT_CHECK(z_get(z_loan(a), z_loan(adm), "", z_move(rcb), NULL) == Z_OK, "query the router's admin space @/*/router");
  for (int i = 0; i < 50 && !replies; i++) usleep(100000);
  char m[200];
  snprintf(m, sizeof m, "zenohd answered (%s) with its zid and listener tcp/127.0.0.1:17457", rkey);
  QT_CHECK(replies == 1 && router_ok && !strncmp(rkey, "@/", 2), m);

  kill(pid, SIGTERM);
  int st = 0;
  waitpid(pid, &st, 0);
  received = 0;
  for (int i = 0; i < 10; i++) {
    z_owned_bytes_t payload;
    z_bytes_copy_from_str(&payload, "after the router stopped");
    z_put(z_loan(a), z_loan(ke), z_move(payload), NULL);
    usleep(100000);
  }
  QT_CHECK(received == 0, "with zenohd stopped, nothing reaches B (no direct client-to-client path)");
  z_drop(z_move(s));
  z_drop(z_move(b));
  z_drop(z_move(a));
  QT_END();
}
