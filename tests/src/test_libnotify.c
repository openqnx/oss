/* libnotify: notification construction and the D-Bus proxy. A target with no
 * notification daemon cannot show anything, so the send path is checked for a
 * clean error rather than success. */
#include "common.h"
#include <libnotify/notify.h>
int main(void) {
  QT_BEGIN("libnotify");
  QT_CHECK(notify_init("qnx-port-test"), "notify_init");
  QT_CHECK(notify_is_initted(), "notify_is_initted");
  QT_CHECK(g_strcmp0(notify_get_app_name(), "qnx-port-test") == 0, "app name");
  NotifyNotification *n = notify_notification_new("Port build", "123 packages staged",
                                                  "dialog-information");
  QT_CHECK(n != NULL, "notify_notification_new");
  notify_notification_set_timeout(n, 3000);
  notify_notification_set_urgency(n, NOTIFY_URGENCY_NORMAL);
  notify_notification_set_category(n, "device");
  notify_notification_add_action(n, "open", "Open", NULL, NULL, NULL);
  QT_CHECK(notify_notification_update(n, "Port build", "done", NULL),
           "notify_notification_update");
  GError *err = NULL;
  gboolean sent = notify_notification_show(n, &err);
  if (sent) {
    printf("  info a notification daemon accepted the message\n");
    notify_notification_close(n, NULL);
  } else {
    printf("  info no notification daemon: %s\n", err ? err->message : "(no message)");
    QT_CHECK(err != NULL, "the absence of a daemon is reported as an error");
    g_clear_error(&err);
  }
  g_object_unref(n);
  notify_uninit();
  QT_CHECK(!notify_is_initted(), "notify_uninit");
  QT_END();
}
