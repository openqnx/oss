/* libsecret: schema handling and the D-Bus proxy to the Secret Service. No
 * keyring daemon runs on a bare target, so the lookup path is checked for a
 * clean error rather than a stored value. */
#include "common.h"
#include <libsecret/secret.h>
static const SecretSchema schema = {
  "org.qnx.PortTest", SECRET_SCHEMA_NONE,
  { { "arch", SECRET_SCHEMA_ATTRIBUTE_STRING },
    { "slot", SECRET_SCHEMA_ATTRIBUTE_INTEGER },
    { NULL, 0 } }
};
int main(void) {
  QT_BEGIN("libsecret");
  QT_CHECK(schema.name != NULL, "schema is usable");
  SecretValue *v = secret_value_new("hunter2", -1, "text/plain");
  QT_CHECK(v != NULL, "secret_value_new");
  gsize len = 0;
  const gchar *txt = secret_value_get(v, &len);
  QT_CHECK(txt && len == 7 && strcmp(txt, "hunter2") == 0, "value round trip");
  QT_CHECK(g_strcmp0(secret_value_get_content_type(v), "text/plain") == 0,
           "content type preserved");
  SecretValue *ref = secret_value_ref(v);
  QT_CHECK(ref == v, "secret_value_ref");
  secret_value_unref(ref);
  secret_value_unref(v);

  /* The service proxy needs a session bus and a keyring daemon; on a bare
     target neither exists, and the failure must be reported, not crash. */
  GError *err = NULL;
  SecretService *svc = secret_service_get_sync(SECRET_SERVICE_NONE, NULL, &err);
  if (svc) {
    printf("  info a Secret Service is running\n");
    QT_CHECK(SECRET_IS_SERVICE(svc), "got a service proxy");
    g_object_unref(svc);
  } else {
    printf("  info no Secret Service: %s\n", err ? err->message : "(no message)");
    QT_CHECK(err != NULL, "absence of the service is reported as an error");
    g_clear_error(&err);
  }
  QT_END();
}
