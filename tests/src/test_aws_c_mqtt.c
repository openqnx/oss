/* aws-c-mqtt: the MQTT 3.1.1 client against a minimal broker written here
 * (one thread, plain sockets on 127.0.0.1). The broker checks the client's
 * CONNECT, SUBSCRIBE and QoS 1 PUBLISH packets byte by byte, acknowledges
 * them and echoes the publish back, which the client must deliver to its
 * subscription callback. */
#include "common.h"
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <aws/common/common.h>
#include <aws/common/condition_variable.h>
#include <aws/common/mutex.h>
#include <aws/io/io.h>
#include <aws/io/event_loop.h>
#include <aws/io/channel_bootstrap.h>
#include <aws/io/host_resolver.h>
#include <aws/io/socket.h>
#include <aws/mqtt/mqtt.h>
#include <aws/mqtt/client.h>

/* ---- the broker ---- */
static int lfd, broker_port;
static struct { int connect_ok, sub_ok, pub_ok, disconnect; char client_id[32]; int keepalive; } bk;
static int read_n(int fd, unsigned char *b, size_t n) {
  size_t got = 0;
  while (got < n) { ssize_t r = read(fd, b + got, n - got); if (r <= 0) return -1; got += (size_t)r; }
  return 0;
}
static void write_all(int fd, const unsigned char *b, size_t n) {
  while (n) { ssize_t w = write(fd, b, n); if (w <= 0) return; b += w; n -= (size_t)w; }
}
static void *broker(void *arg) {
  (void)arg;
  int fd = accept(lfd, NULL, NULL);
  if (fd < 0) return NULL;
  unsigned char pkt[1024];
  for (;;) {
    unsigned char h;
    if (read_n(fd, &h, 1)) break;
    size_t len = 0; int shift = 0; unsigned char c;
    do { if (read_n(fd, &c, 1)) goto out; len |= (size_t)(c & 0x7f) << shift; shift += 7; } while (c & 0x80);
    if (len > sizeof pkt || read_n(fd, pkt, len)) break;
    switch (h >> 4) {
      case 1: { /* CONNECT: "MQTT", level 4, flags, keepalive, client id */
        if (len >= 12 && pkt[0] == 0 && pkt[1] == 4 && memcmp(pkt + 2, "MQTT", 4) == 0 && pkt[6] == 4) {
          bk.keepalive = (pkt[8] << 8) | pkt[9];
          size_t idl = (size_t)((pkt[10] << 8) | pkt[11]);
          if (idl < sizeof bk.client_id && 12 + idl <= len) { memcpy(bk.client_id, pkt + 12, idl); bk.client_id[idl] = 0; }
          bk.connect_ok = (pkt[7] & 0x02) != 0;   /* clean session */
        }
        unsigned char ack[] = {0x20, 0x02, 0x00, 0x00};
        write_all(fd, ack, 4);
        break;
      }
      case 8: { /* SUBSCRIBE */
        size_t tl = (size_t)((pkt[2] << 8) | pkt[3]);
        bk.sub_ok = (h & 0x0f) == 2 && tl == 10 && memcmp(pkt + 4, "qnx/test/#", 10) == 0 && pkt[4 + tl] == 1;
        unsigned char ack[] = {0x90, 0x03, pkt[0], pkt[1], 0x01};
        write_all(fd, ack, 5);
        break;
      }
      case 3: { /* PUBLISH */
        int qos = (h >> 1) & 3;
        size_t tl = (size_t)((pkt[0] << 8) | pkt[1]);
        size_t off = 2 + tl + (qos ? 2 : 0);
        if (qos == 1) {
          unsigned char ack[] = {0x40, 0x02, pkt[2 + tl], pkt[3 + tl]};
          write_all(fd, ack, 4);
        }
        bk.pub_ok = qos == 1 && tl == 14 && memcmp(pkt + 2, "qnx/test/hello", 14) == 0 &&
                    len - off == 11 && memcmp(pkt + off, "hi from qnx", 11) == 0;
        /* echo to the subscriber at QoS 0 */
        unsigned char echo[256]; size_t el = 2 + tl + (len - off);
        echo[0] = 0x30; echo[1] = (unsigned char)el;
        memcpy(echo + 2, pkt, 2 + tl);
        memcpy(echo + 4 + tl, pkt + off, len - off);
        write_all(fd, echo, 2 + el);
        break;
      }
      case 12: { unsigned char r[] = {0xD0, 0x00}; write_all(fd, r, 2); break; }  /* PINGREQ */
      case 10: { unsigned char r[] = {0xB0, 0x02, pkt[0], pkt[1]}; write_all(fd, r, 4); break; }  /* UNSUBSCRIBE */
      case 14: bk.disconnect = 1; goto out;
      default: break;
    }
  }
out:
  close(fd);
  return NULL;
}

/* ---- the client ---- */
static struct aws_mutex mtx = AWS_MUTEX_INIT;
static struct aws_condition_variable cv = AWS_CONDITION_VARIABLE_INIT;
static int flags, want;
enum { F_CONN = 1, F_SUBACK = 2, F_PUBACK = 4, F_MSG = 8, F_DISC = 16, F_ERR = 32 };
static void set_flag(int f) { aws_mutex_lock(&mtx); flags |= f; aws_condition_variable_notify_all(&cv); aws_mutex_unlock(&mtx); }
static bool pred(void *ud) { (void)ud; return (flags & want) == want || (flags & F_ERR); }
static int wait_flags(int f) {
  aws_mutex_lock(&mtx); want = f;
  aws_condition_variable_wait_for_pred(&cv, &mtx, 10LL * 1000000000LL, pred, NULL);
  int ok = (flags & f) == f; aws_mutex_unlock(&mtx); return ok;
}
static int suback_qos = -1;
static char got_topic[64], got_payload[64];
static void on_conn(struct aws_mqtt_client_connection *c, int err, enum aws_mqtt_connect_return_code rc, bool sp, void *ud) {
  (void)c; (void)sp; (void)ud;
  if (err || rc != AWS_MQTT_CONNECT_ACCEPTED) { printf("  info connect failed: %s rc=%d\n", aws_error_name(err), (int)rc); set_flag(F_ERR); return; }
  set_flag(F_CONN);
}
static void on_suback(struct aws_mqtt_client_connection *c, uint16_t id, const struct aws_byte_cursor *t, enum aws_mqtt_qos q, int err, void *ud) {
  (void)c; (void)id; (void)t; (void)ud;
  if (!err) { suback_qos = (int)q; set_flag(F_SUBACK); } else set_flag(F_ERR);
}
static void on_msg(struct aws_mqtt_client_connection *c, const struct aws_byte_cursor *t, const struct aws_byte_cursor *p,
                   bool dup, enum aws_mqtt_qos q, bool retain, void *ud) {
  (void)c; (void)dup; (void)q; (void)retain; (void)ud;
  snprintf(got_topic, sizeof got_topic, "%.*s", (int)t->len, (const char *)t->ptr);
  snprintf(got_payload, sizeof got_payload, "%.*s", (int)p->len, (const char *)p->ptr);
  set_flag(F_MSG);
}
static void on_puback(struct aws_mqtt_client_connection *c, uint16_t id, int err, void *ud) {
  (void)c; (void)id; (void)ud;
  set_flag(err ? F_ERR : F_PUBACK);
}
static void on_disc(struct aws_mqtt_client_connection *c, void *ud) { (void)c; (void)ud; set_flag(F_DISC); }

int main(void) {
  QT_BEGIN("aws-c-mqtt");
  alarm(60);
  struct aws_allocator *al = aws_default_allocator();
  aws_mqtt_library_init(al);

  lfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sa; memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET; sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); sa.sin_port = 0;
  socklen_t sl = sizeof sa;
  int lok = bind(lfd, (struct sockaddr *)&sa, sizeof sa) == 0 && listen(lfd, 1) == 0 &&
            getsockname(lfd, (struct sockaddr *)&sa, &sl) == 0;
  broker_port = ntohs(sa.sin_port);
  QT_CHECK(lok && broker_port > 0, "test broker listening on 127.0.0.1");
  printf("  info broker port %d\n", broker_port);
  pthread_t bt;
  pthread_create(&bt, NULL, broker, NULL);

  struct aws_event_loop_group_options eo = {0};
  eo.loop_count = 1;
  struct aws_event_loop_group *elg = aws_event_loop_group_new(al, &eo);
  struct aws_host_resolver_default_options ro = {0};
  ro.max_entries = 8; ro.el_group = elg;
  struct aws_host_resolver *res = aws_host_resolver_new_default(al, &ro);
  struct aws_client_bootstrap_options cbo = {0};
  cbo.event_loop_group = elg; cbo.host_resolver = res;
  struct aws_client_bootstrap *cb = aws_client_bootstrap_new(al, &cbo);
  struct aws_mqtt_client *cli = aws_mqtt_client_new(al, cb);
  struct aws_mqtt_client_connection *conn = cli ? aws_mqtt_client_connection_new(cli) : NULL;
  QT_CHECK(conn != NULL, "MQTT client and connection objects");
  if (!conn) QT_END();

  struct aws_socket_options so = {0};
  so.type = AWS_SOCKET_STREAM; so.domain = AWS_SOCKET_IPV4; so.connect_timeout_ms = 5000;
  struct aws_mqtt_connection_options co = {0};
  co.host_name = aws_byte_cursor_from_c_str("127.0.0.1");
  co.port = (uint32_t)broker_port;
  co.socket_options = &so;
  co.client_id = aws_byte_cursor_from_c_str("qnx-port-test");
  co.keep_alive_time_secs = 30;
  co.ping_timeout_ms = 3000;
  co.on_connection_complete = on_conn;
  co.clean_session = true;
  QT_CHECK(aws_mqtt_client_connection_connect(conn, &co) == AWS_OP_SUCCESS, "connect started");
  QT_CHECK(wait_flags(F_CONN), "CONNACK accepted");
  QT_CHECK(bk.connect_ok && strcmp(bk.client_id, "qnx-port-test") == 0 && bk.keepalive == 30,
           "broker saw CONNECT: MQTT 3.1.1, clean session, client id, keep-alive 30");

  struct aws_byte_cursor filt = aws_byte_cursor_from_c_str("qnx/test/#");
  QT_CHECK(aws_mqtt_client_connection_subscribe(conn, &filt, AWS_MQTT_QOS_AT_LEAST_ONCE, on_msg, NULL, NULL, on_suback, NULL) != 0,
           "subscribe queued");
  QT_CHECK(wait_flags(F_SUBACK) && suback_qos == 1, "SUBACK grants QoS 1");
  QT_CHECK(bk.sub_ok, "broker saw SUBSCRIBE qnx/test/# at QoS 1");

  struct aws_byte_cursor topic = aws_byte_cursor_from_c_str("qnx/test/hello");
  struct aws_byte_cursor payload = aws_byte_cursor_from_c_str("hi from qnx");
  QT_CHECK(aws_mqtt_client_connection_publish(conn, &topic, AWS_MQTT_QOS_AT_LEAST_ONCE, false, &payload, on_puback, NULL) != 0,
           "publish queued");
  QT_CHECK(wait_flags(F_PUBACK), "PUBACK completes the QoS 1 publish");
  QT_CHECK(bk.pub_ok, "broker saw PUBLISH with topic and payload");
  QT_CHECK(wait_flags(F_MSG), "echoed message delivered to the subscription");
  QT_CHECK(strcmp(got_topic, "qnx/test/hello") == 0 && strcmp(got_payload, "hi from qnx") == 0, "delivered topic and payload");

  aws_mqtt_client_connection_disconnect(conn, on_disc, NULL);
  QT_CHECK(wait_flags(F_DISC), "disconnect completes");
  pthread_join(bt, NULL);
  QT_CHECK(bk.disconnect, "broker saw DISCONNECT");
  close(lfd);
  aws_mqtt_client_connection_release(conn);
  aws_mqtt_client_release(cli);
  aws_client_bootstrap_release(cb);
  aws_host_resolver_release(res);
  aws_event_loop_group_release(elg);
  aws_mqtt_library_clean_up();
  QT_END();
}
