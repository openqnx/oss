/* paho.mqtt.c against a minimal MQTT 3.1.1 broker running in a thread of
 * this test on 127.0.0.1 (CONNECT/CONNACK, SUBSCRIBE/SUBACK, PUBLISH with
 * QoS 0 and 1 echoed to the subscriber, PUBACK, PINGREQ/PINGRESP,
 * UNSUBSCRIBE/UNSUBACK, DISCONNECT). The synchronous MQTTClient API
 * connects, subscribes, publishes and receives its own messages back; the
 * asynchronous MQTTAsync API does the same through callbacks. The broker
 * checks what arrives on the wire (protocol level 4, client IDs, keepalive,
 * topic and payload bytes). MQTTVersion reports the library version. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <MQTTClient.h>
#include <MQTTAsync.h>

#define TOPIC "qnx/test/topic"
#define PAYLOAD1 "hello from QNX, qos 1"
#define PAYLOAD0 "and again with qos 0"

/* ---- the broker -------------------------------------------------------- */
static int lsock = -1, broker_port = 0;
static volatile int broker_stop = 0;
static struct {
  int connects, protocol_level, keepalive, subscribes, publishes, pubacks_from_client,
      pings, unsubscribes, disconnects, bad_packets;
  char client_ids[2][64];
  char last_topic[128], last_payload[128];
} seen;

static int read_full(int fd, unsigned char *b, int n) {
  int got = 0;
  while (got < n) {
    struct pollfd p = { fd, POLLIN, 0 };
    if (poll(&p, 1, 200) <= 0) { if (broker_stop) return -1; continue; }
    ssize_t r = recv(fd, b + got, n - got, 0);
    if (r <= 0) return -1;
    got += (int)r;
  }
  return got;
}
static int send_all(int fd, const unsigned char *b, int n) {
  return send(fd, b, n, 0) == n ? 0 : -1;
}
/* Fixed header + remaining length (variable byte integer). */
static int read_packet(int fd, unsigned char *type, unsigned char *buf, int cap) {
  unsigned char h;
  if (read_full(fd, &h, 1) != 1) return -1;
  int len = 0, mul = 1;
  for (int i = 0; i < 4; i++) {
    unsigned char c;
    if (read_full(fd, &c, 1) != 1) return -1;
    len += (c & 127) * mul; mul *= 128;
    if (!(c & 128)) break;
  }
  if (len > cap) return -1;
  if (len && read_full(fd, buf, len) != len) return -1;
  *type = h;
  return len;
}
static int put_len(unsigned char *p, int len) {
  int n = 0;
  do { unsigned char c = len % 128; len /= 128; if (len) c |= 128; p[n++] = c; } while (len);
  return n;
}
static void serve(int fd) {
  unsigned char buf[4096], t, out[4200];
  int len, subqos = -1;
  unsigned short next_id = 1;
  while (!broker_stop && (len = read_packet(fd, &t, buf, sizeof buf)) >= 0) {
    switch (t >> 4) {
    case 1: { /* CONNECT: "MQTT", level, flags, keepalive, client id */
      int pl = buf[0] << 8 | buf[1];
      int o = 2 + pl;
      if (pl != 4 || memcmp(buf + 2, "MQTT", 4)) seen.bad_packets++;
      seen.protocol_level = buf[o];
      if (seen.connects == 0) seen.keepalive = buf[o + 2] << 8 | buf[o + 3];
      int cl = buf[o + 4] << 8 | buf[o + 5];
      if (seen.connects < 2) snprintf(seen.client_ids[seen.connects], 64, "%.*s", cl, buf + o + 6);
      seen.connects++;
      unsigned char ack[] = { 0x20, 0x02, 0x00, 0x00 };
      send_all(fd, ack, 4);
      break;
    }
    case 8: { /* SUBSCRIBE: id, (topic, qos)+ */
      int tl = buf[2] << 8 | buf[3];
      subqos = buf[4 + tl] & 3;
      if (subqos > 1) subqos = 1;
      if (tl != (int)strlen(TOPIC) || memcmp(buf + 4, TOPIC, tl)) seen.bad_packets++;
      seen.subscribes++;
      unsigned char ack[] = { 0x90, 0x03, buf[0], buf[1], (unsigned char)subqos };
      send_all(fd, ack, 5);
      break;
    }
    case 3: { /* PUBLISH */
      int qos = (t >> 1) & 3, tl = buf[0] << 8 | buf[1], o = 2 + tl;
      unsigned char id[2] = { 0, 0 };
      if (qos) { id[0] = buf[o]; id[1] = buf[o + 1]; o += 2; }
      seen.publishes++;
      snprintf(seen.last_topic, sizeof seen.last_topic, "%.*s", tl, buf + 2);
      snprintf(seen.last_payload, sizeof seen.last_payload, "%.*s", len - o, buf + o);
      if (qos == 1) { unsigned char ack[] = { 0x40, 0x02, id[0], id[1] }; send_all(fd, ack, 4); }
      if (subqos >= 0) { /* echo to the subscriber, at min(sub, pub) QoS */
        int q = qos < subqos ? qos : subqos, n = 0, body = 2 + tl + (q ? 2 : 0) + (len - o);
        out[n++] = (unsigned char)(0x30 | q << 1);
        n += put_len(out + n, body);
        memcpy(out + n, buf, 2 + tl); n += 2 + tl;
        if (q) { out[n++] = next_id >> 8; out[n++] = next_id & 255; next_id++; }
        memcpy(out + n, buf + o, len - o); n += len - o;
        send_all(fd, out, n);
      }
      break;
    }
    case 4: seen.pubacks_from_client++; break;
    case 10: { /* UNSUBSCRIBE */
      seen.unsubscribes++; subqos = -1;
      unsigned char ack[] = { 0xb0, 0x02, buf[0], buf[1] };
      send_all(fd, ack, 4);
      break;
    }
    case 12: { seen.pings++; unsigned char r[] = { 0xd0, 0x00 }; send_all(fd, r, 2); break; }
    case 14: seen.disconnects++; return;
    default: seen.bad_packets++; return;
    }
  }
}
static void *broker(void *arg) {
  (void)arg;
  while (!broker_stop) {
    struct pollfd p = { lsock, POLLIN, 0 };
    if (poll(&p, 1, 200) <= 0) continue;
    int fd = accept(lsock, NULL, NULL);
    if (fd < 0) continue;
    serve(fd);
    close(fd);
  }
  return NULL;
}
static int start_broker(void) {
  lsock = socket(AF_INET, SOCK_STREAM, 0);
  if (lsock < 0) return -1;
  int one = 1;
  setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  struct sockaddr_in a;
  memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK); a.sin_port = 0;
  socklen_t al = sizeof a;
  if (bind(lsock, (struct sockaddr *)&a, sizeof a) || listen(lsock, 4) ||
      getsockname(lsock, (struct sockaddr *)&a, &al))
    return -1;
  broker_port = ntohs(a.sin_port);
  return 0;
}

/* ---- the asynchronous client's callbacks ---------------------------------- */
static volatile int a_connected, a_subscribed, a_sent, a_failed, a_arrived;
static char a_payload[128];
static void a_onconnect(void *c, MQTTAsync_successData *r) { (void)c; (void)r; a_connected = 1; }
static void a_onsubscribe(void *c, MQTTAsync_successData *r) { (void)c; (void)r; a_subscribed = 1; }
static void a_onsend(void *c, MQTTAsync_successData *r) { (void)c; (void)r; a_sent = 1; }
static void a_onfailure(void *c, MQTTAsync_failureData *r) { (void)c; (void)r; a_failed = 1; }
static int a_msgarrived(void *c, char *topic, int tlen, MQTTAsync_message *m) {
  (void)c; (void)tlen;
  snprintf(a_payload, sizeof a_payload, "%.*s", m->payloadlen, (char *)m->payload);
  a_arrived++;
  MQTTAsync_freeMessage(&m);
  MQTTAsync_free(topic);
  return 1;
}
static int wait_flag(volatile int *f, int ms) {
  for (int i = 0; i < ms / 10 && !*f && !a_failed; i++) usleep(10000);
  return *f;
}

int main(void) {
  QT_BEGIN("paho.mqtt.c");
  MQTTClient_nameValue *vi = MQTTClient_getVersionInfo();
  for (; vi && vi->name; vi++)
    if (strcmp(vi->name, "Version") == 0) printf("  info paho.mqtt.c %s\n", vi->value ? vi->value : "?");
  QT_CHECK(start_broker() == 0, "test broker listening on 127.0.0.1");
  printf("  info broker on port %d\n", broker_port);
  pthread_t th;
  QT_CHECK(pthread_create(&th, NULL, broker, NULL) == 0, "broker thread started");
  char uri[64];
  snprintf(uri, sizeof uri, "tcp://127.0.0.1:%d", broker_port);

  /* Synchronous client. */
  MQTTClient c = NULL;
  int rc = MQTTClient_create(&c, uri, "qnx-sync-client", MQTTCLIENT_PERSISTENCE_NONE, NULL);
  QT_CHECK(rc == MQTTCLIENT_SUCCESS && c, "MQTTClient_create");
  MQTTClient_connectOptions co = MQTTClient_connectOptions_initializer;
  co.keepAliveInterval = 17; co.cleansession = 1; co.connectTimeout = 5;
  co.MQTTVersion = MQTTVERSION_3_1_1;
  rc = MQTTClient_connect(c, &co);
  QT_CHECK(rc == MQTTCLIENT_SUCCESS, "MQTTClient_connect (CONNACK accepted)");
  QT_CHECK(MQTTClient_isConnected(c), "MQTTClient_isConnected");
  rc = MQTTClient_subscribe(c, TOPIC, 1);
  QT_CHECK(rc == MQTTCLIENT_SUCCESS, "MQTTClient_subscribe at QoS 1 (SUBACK)");

  MQTTClient_message m = MQTTClient_message_initializer;
  m.payload = (void *)PAYLOAD1; m.payloadlen = (int)strlen(PAYLOAD1); m.qos = 1;
  MQTTClient_deliveryToken tok = 0;
  rc = MQTTClient_publishMessage(c, TOPIC, &m, &tok);
  QT_CHECK(rc == MQTTCLIENT_SUCCESS, "MQTTClient_publishMessage QoS 1");
  rc = MQTTClient_waitForCompletion(c, tok, 5000);
  QT_CHECK(rc == MQTTCLIENT_SUCCESS, "QoS 1 delivery completed (PUBACK)");

  for (int round = 0; round < 2; round++) {
    const char *want = round == 0 ? PAYLOAD1 : PAYLOAD0;
    if (round == 1) {
      rc = MQTTClient_publish(c, TOPIC, (int)strlen(PAYLOAD0), PAYLOAD0, 0, 0, NULL);
      QT_CHECK(rc == MQTTCLIENT_SUCCESS, "MQTTClient_publish QoS 0");
    }
    char *topic = NULL; int tlen = 0; MQTTClient_message *in = NULL;
    for (int i = 0; i < 10 && !in; i++) MQTTClient_receive(c, &topic, &tlen, &in, 500);
    int ok = in && topic && strcmp(topic, TOPIC) == 0 && in->payloadlen == (int)strlen(want) &&
             memcmp(in->payload, want, in->payloadlen) == 0;
    if (in) printf("  info received on %s: \"%.*s\" qos %d\n", topic ? topic : "?", in->payloadlen,
                   (char *)in->payload, in->qos);
    QT_CHECK(ok, round == 0 ? "QoS 1 message echoed back to the subscriber" : "QoS 0 message echoed back");
    if (round == 0) QT_CHECK(in && in->qos == 1, "echoed message carries QoS 1");
    if (in) MQTTClient_freeMessage(&in);
    if (topic) MQTTClient_free(topic);
  }
  rc = MQTTClient_unsubscribe(c, TOPIC);
  QT_CHECK(rc == MQTTCLIENT_SUCCESS, "MQTTClient_unsubscribe (UNSUBACK)");
  rc = MQTTClient_disconnect(c, 1000);
  QT_CHECK(rc == MQTTCLIENT_SUCCESS, "MQTTClient_disconnect");
  MQTTClient_destroy(&c);

  /* Asynchronous client, second connection to the same broker. */
  MQTTAsync ac = NULL;
  rc = MQTTAsync_create(&ac, uri, "qnx-async-client", MQTTCLIENT_PERSISTENCE_NONE, NULL);
  QT_CHECK(rc == MQTTASYNC_SUCCESS && ac, "MQTTAsync_create");
  MQTTAsync_setCallbacks(ac, NULL, NULL, a_msgarrived, NULL);
  MQTTAsync_connectOptions aco = MQTTAsync_connectOptions_initializer;
  aco.keepAliveInterval = 20; aco.cleansession = 1; aco.MQTTVersion = MQTTVERSION_3_1_1;
  aco.onSuccess = a_onconnect; aco.onFailure = a_onfailure; aco.connectTimeout = 5;
  rc = MQTTAsync_connect(ac, &aco);
  QT_CHECK(rc == MQTTASYNC_SUCCESS && wait_flag(&a_connected, 5000), "MQTTAsync_connect -> onSuccess");
  MQTTAsync_responseOptions ro = MQTTAsync_responseOptions_initializer;
  ro.onSuccess = a_onsubscribe; ro.onFailure = a_onfailure;
  rc = MQTTAsync_subscribe(ac, TOPIC, 1, &ro);
  QT_CHECK(rc == MQTTASYNC_SUCCESS && wait_flag(&a_subscribed, 5000), "MQTTAsync_subscribe -> onSuccess");
  MQTTAsync_responseOptions so = MQTTAsync_responseOptions_initializer;
  so.onSuccess = a_onsend; so.onFailure = a_onfailure;
  MQTTAsync_message am = MQTTAsync_message_initializer;
  am.payload = (void *)"async payload"; am.payloadlen = 13; am.qos = 1;
  rc = MQTTAsync_sendMessage(ac, TOPIC, &am, &so);
  QT_CHECK(rc == MQTTASYNC_SUCCESS && wait_flag(&a_sent, 5000), "MQTTAsync_sendMessage QoS 1 -> onSuccess");
  QT_CHECK(wait_flag(&a_arrived, 5000) && strcmp(a_payload, "async payload") == 0,
           "messageArrived callback got \"async payload\"");
  MQTTAsync_disconnectOptions dopt = MQTTAsync_disconnectOptions_initializer;
  dopt.timeout = 1000;
  MQTTAsync_disconnect(ac, &dopt);
  for (int i = 0; i < 100 && MQTTAsync_isConnected(ac); i++) usleep(10000);
  MQTTAsync_destroy(&ac);

  usleep(200000);
  broker_stop = 1;
  pthread_join(th, NULL);
  close(lsock);

  printf("  info broker saw: %d connect(s) [%s, %s], level %d, keepalive %d, %d subscribe(s), "
         "%d publish(es), %d puback(s) from clients, %d unsubscribe(s), %d disconnect(s), %d bad\n",
         seen.connects, seen.client_ids[0], seen.client_ids[1], seen.protocol_level, seen.keepalive,
         seen.subscribes, seen.publishes, seen.pubacks_from_client, seen.unsubscribes, seen.disconnects,
         seen.bad_packets);
  QT_CHECK(seen.connects == 2 && strcmp(seen.client_ids[0], "qnx-sync-client") == 0 &&
           strcmp(seen.client_ids[1], "qnx-async-client") == 0, "broker: two CONNECTs with our client IDs");
  QT_CHECK(seen.protocol_level == 4, "broker: MQTT 3.1.1 (protocol level 4)");
  QT_CHECK(seen.keepalive == 17, "broker: keepalive 17 s as set in the connect options");
  QT_CHECK(seen.subscribes == 2 && seen.publishes == 3 && seen.unsubscribes == 1, "broker: 2 SUBSCRIBE, 3 PUBLISH, 1 UNSUBSCRIBE");
  QT_CHECK(seen.pubacks_from_client >= 2, "broker: clients acknowledged the QoS 1 echoes");
  QT_CHECK(strcmp(seen.last_payload, "async payload") == 0 && strcmp(seen.last_topic, TOPIC) == 0,
           "broker: topic and payload bytes as sent");
  QT_CHECK(seen.disconnects == 2 && seen.bad_packets == 0, "broker: clean DISCONNECTs, no malformed packets");

  char out[2048];
  FILE *p = popen("MQTTVersion 2>&1", "r");
  size_t n = p ? fread(out, 1, sizeof out - 1, p) : 0;
  out[n] = 0;
  int st = p ? pclose(p) : -1;
  printf("  info MQTTVersion: %.200s\n", out);
  QT_CHECK(WIFEXITED(st) && strstr(out, "1.3.14") != NULL, "MQTTVersion reports 1.3.14");
  QT_END();
}
