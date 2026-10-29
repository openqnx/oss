/* mosquitto: client library init, version, and TLS-capable client object. */
#include "common.h"
#include <mosquitto.h>
int main(void) {
  QT_BEGIN("mosquitto");
  int maj, min, rev;
  QT_CHECK(mosquitto_lib_init() == MOSQ_ERR_SUCCESS, "lib init");
  mosquitto_lib_version(&maj, &min, &rev);
  QT_CHECK(maj == 2, "libmosquitto 2.x");
  struct mosquitto *m = mosquitto_new("qnx-test", true, NULL);
  QT_CHECK(m != NULL, "client object");
  QT_CHECK(mosquitto_tls_opts_set(m, 1, NULL, NULL) == MOSQ_ERR_SUCCESS, "TLS support compiled in");
  QT_CHECK(mosquitto_connect(m, "127.0.0.1", 1, 1) != MOSQ_ERR_SUCCESS, "connect to closed port fails cleanly");
  mosquitto_destroy(m); mosquitto_lib_cleanup();
  QT_END();
}
