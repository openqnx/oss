#include "common.h"
#include <MQTTClient.h>
#include <MQTTAsync.h>
/* Client handle lifecycle and message construction -- no broker needed. */
int main(void) {
  QT_BEGIN("paho.mqtt.c");
  MQTTClient client = NULL;
  int rc = MQTTClient_create(&client, "tcp://127.0.0.1:1883", "qnx_port_test",
                             MQTTCLIENT_PERSISTENCE_NONE, NULL);
  QT_CHECK(rc == MQTTCLIENT_SUCCESS, "MQTTClient_create");
  QT_CHECK(client != NULL, "client handle non-null");
  MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
  QT_CHECK(opts.keepAliveInterval > 0, "connectOptions initializer populated");
  MQTTClient_message msg = MQTTClient_message_initializer;
  msg.payload = (void *)"telemetry"; msg.payloadlen = 9; msg.qos = 1;
  QT_CHECK(msg.payloadlen == 9 && msg.qos == 1, "message initializer");
  MQTTClient_destroy(&client);
  QT_CHECK(1, "MQTTClient_destroy");
  printf("  info paho.mqtt.c %s\n", MQTTClient_getVersionInfo()[0].value);
  QT_END();
}
