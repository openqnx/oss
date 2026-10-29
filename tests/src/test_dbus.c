/* D-Bus: message construction and marshalling, plus the address parser and
 * the server/transport setup. No bus daemon is expected to be running, so the
 * connection attempt is checked for a clean error rather than success. */
#include "common.h"
#include <stdlib.h>
#include <dbus/dbus.h>
int main(void) {
  QT_BEGIN("dbus");
  int maj = 0, min = 0, mic = 0;
  dbus_get_version(&maj, &min, &mic);
  printf("  info dbus %d.%d.%d\n", maj, min, mic);
  DBusError err;
  dbus_error_init(&err);

  DBusMessage *m = dbus_message_new_method_call(
      "com.example.Ports", "/com/example/Ports", "com.example.Ports", "Build");
  QT_CHECK(m != NULL, "dbus_message_new_method_call");
  DBusMessageIter it;
  dbus_message_iter_init_append(m, &it);
  const char *arch = "aarch64le";
  dbus_int32_t sdp = 8;
  QT_CHECK(dbus_message_iter_append_basic(&it, DBUS_TYPE_STRING, &arch),
           "append a string argument");
  QT_CHECK(dbus_message_iter_append_basic(&it, DBUS_TYPE_INT32, &sdp),
           "append an int32 argument");
  QT_CHECK(strcmp(dbus_message_get_member(m), "Build") == 0, "member name");
  QT_CHECK(dbus_message_get_type(m) == DBUS_MESSAGE_TYPE_METHOD_CALL, "message type");

  /* Marshal to the wire format and parse it back -- this is the byte-order
   * and alignment code that a new architecture is most likely to break. */
  /* An unsent message has serial 0, which is not a valid wire value, so
   * demarshalling it fails. Assign one the way a connection would. */
  dbus_message_set_serial(m, 1);
  char *blob = NULL; int blen = 0;
  QT_CHECK(dbus_message_marshal(m, &blob, &blen) && blen > 0, "dbus_message_marshal");
  QT_CHECK(dbus_message_demarshal_bytes_needed(blob, blen) == blen,
           "demarshal_bytes_needed agrees with the marshalled size");
  DBusMessage *back = dbus_message_demarshal(blob, blen, &err);
  QT_CHECK(back != NULL, "dbus_message_demarshal");
  if (back) {
    const char *got_arch = NULL; dbus_int32_t got_sdp = 0;
    QT_CHECK(dbus_message_get_args(back, &err, DBUS_TYPE_STRING, &got_arch,
                                   DBUS_TYPE_INT32, &got_sdp, DBUS_TYPE_INVALID),
             "dbus_message_get_args");
    QT_CHECK(got_arch && strcmp(got_arch, "aarch64le") == 0 && got_sdp == 8,
             "arguments survive the round trip");
    dbus_message_unref(back);
  }
  dbus_free(blob);
  dbus_message_unref(m);

  QT_CHECK(dbus_validate_bus_name("com.example.Ports", NULL), "bus name validation");
  QT_CHECK(!dbus_validate_bus_name("0bad name", NULL), "invalid bus name rejected");
  QT_CHECK(dbus_validate_path("/com/example/Ports", NULL), "object path validation");

  /* A private connection to a well-formed but unreachable address must fail
   * cleanly through the socket transport rather than hang or abort. */
  dbus_error_free(&err);
  dbus_error_init(&err);
  DBusConnection *c = dbus_connection_open_private("unix:path=/tmp/qnx-port-no-such-bus",
                                                   &err);
  QT_CHECK(c == NULL && dbus_error_is_set(&err), "unreachable bus reports an error");
  printf("  info %s\n", err.message ? err.message : "(no message)");
  dbus_error_free(&err);
  dbus_shutdown();
  QT_END();
}
