#include "common.h"
#include <string.h>
#include <lcm/lcm.h>
/* lcm: publish and subscribe in one process through the memq provider:
 * the subscriber gets the message on its channel, not on another. */
static int got; static char data[64]; static char chan[64];
static void on_msg(const lcm_recv_buf_t *rb, const char *ch, void *u) {
  (void)u; got++; snprintf(chan, sizeof chan, "%s", ch);
  memcpy(data, rb->data, rb->data_size < 63 ? rb->data_size : 63);
}
int main(void) {
  QT_BEGIN("lcm");
  lcm_t *l = lcm_create("memq://");
  QT_CHECK(l != NULL, "lcm_create(memq://)");
  if (l) {
    lcm_subscription_t *s = lcm_subscribe(l, "QNX_.*", on_msg, NULL);
    QT_CHECK(s != NULL, "lcm_subscribe (regex channel)");
    QT_CHECK(lcm_publish(l, "QNX_TEST", "hello lcm", 10) == 0, "lcm_publish");
    QT_CHECK(lcm_publish(l, "OTHER", "ignored", 8) == 0, "publish on another channel");
    QT_CHECK(lcm_handle_timeout(l, 1000) > 0, "lcm_handle_timeout");
    lcm_handle_timeout(l, 200);
    QT_CHECK(got == 1, "exactly one message received");
    QT_CHECK(strcmp(chan, "QNX_TEST") == 0 && strcmp(data, "hello lcm") == 0, "channel and payload");
    lcm_unsubscribe(l, s); lcm_destroy(l);
  }
  QT_END();
}
