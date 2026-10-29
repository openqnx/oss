/* open62541: start a server, connect a client, read a node. */
#include "common.h"
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include <pthread.h>
static volatile UA_Boolean running = true;
static void *serve(void *s) { UA_Server_run((UA_Server *)s, &running); return NULL; }
int main(void) {
  QT_BEGIN("open62541");
  UA_Server *srv = UA_Server_new();
  UA_ServerConfig *cfg = UA_Server_getConfig(srv);
  cfg->logging->context = (void *)(uintptr_t)UA_LOGLEVEL_ERROR;
  QT_CHECK(UA_ServerConfig_setMinimal(cfg, 48400, NULL) == UA_STATUSCODE_GOOD, "server config");
  pthread_t t; pthread_create(&t, NULL, serve, srv);
  UA_Client *c = UA_Client_new();
  UA_ClientConfig_setDefault(UA_Client_getConfig(c));
  UA_StatusCode rc = UA_STATUSCODE_BAD;
  for (int i = 0; i < 50 && rc != UA_STATUSCODE_GOOD; i++) { rc = UA_Client_connect(c, "opc.tcp://127.0.0.1:48400"); if (rc) usleep(100000); }
  QT_CHECK(rc == UA_STATUSCODE_GOOD, "client connects");
  UA_Variant v; UA_Variant_init(&v);
  rc = UA_Client_readValueAttribute(c, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_STATE), &v);
  QT_CHECK(rc == UA_STATUSCODE_GOOD, "reads ServerStatus/State");
  UA_Variant_clear(&v); UA_Client_disconnect(c); UA_Client_delete(c);
  running = false; pthread_join(t, NULL); UA_Server_delete(srv);
  QT_END();
}
