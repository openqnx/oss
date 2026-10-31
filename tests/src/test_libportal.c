/* libportal: the XDG desktop-portal client library, end to end over D-Bus.
 * The test starts a private dbus-daemon, serves a minimal
 * org.freedesktop.portal.Settings implementation itself (its own thread and
 * main context, owning org.freedesktop.portal.Desktop), and drives it
 * through libportal: XdpPortal setup, xdp_settings_read_string/_uint,
 * read_all_values, a NotFound error, and the "changed" signal for a
 * SettingChanged emission. Every wait is bounded; the daemon is stopped by
 * the PID this test started. */
#include "common.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libportal/portal.h>
#include <libportal/settings.h>
extern char **environ;

static const char *DIR_ = "/tmp/qnxport-libportal";
static pid_t bus_pid = -1;

static void stop_bus(void) {
  if (bus_pid <= 0) return;
  int st;
  kill(bus_pid, SIGTERM);
  for (int i = 0; i < 30; ++i) { if (waitpid(bus_pid, &st, WNOHANG) == bus_pid) { bus_pid = -1; return; } usleep(100000); }
  kill(bus_pid, SIGKILL); waitpid(bus_pid, &st, 0); bus_pid = -1;
}
static int start_bus(char *addr, size_t n) {
  char conf[128], sock[128], log[128], text[1024], carg[160];
  mkdir(DIR_, 0700);
  snprintf(sock, sizeof sock, "%s/bus", DIR_);
  snprintf(conf, sizeof conf, "%s/bus.conf", DIR_);
  snprintf(log, sizeof log, "%s/dbus.log", DIR_);
  unlink(sock);
  snprintf(text, sizeof text,
    "<!DOCTYPE busconfig PUBLIC \"-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN\"\n"
    " \"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">\n"
    "<busconfig>\n  <type>session</type>\n  <listen>unix:path=%s</listen>\n  <allow_anonymous/>\n"
    "  <policy context=\"default\">\n    <allow send_destination=\"*\" eavesdrop=\"true\"/>\n"
    "    <allow eavesdrop=\"true\"/>\n    <allow own=\"*\"/>\n  </policy>\n</busconfig>\n", sock);
  FILE *f = fopen(conf, "w");
  if (!f) return 0;
  fputs(text, f); fclose(f);
  snprintf(carg, sizeof carg, "--config-file=%s", conf);
  char *argv[] = {"dbus-daemon", carg, "--nofork", "--nopidfile", NULL};
  posix_spawn_file_actions_t fa;
  posix_spawn_file_actions_init(&fa);
  posix_spawn_file_actions_addopen(&fa, 0, "/dev/null", O_RDONLY, 0);
  posix_spawn_file_actions_addopen(&fa, 1, log, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  posix_spawn_file_actions_adddup2(&fa, 1, 2);
  int rc = posix_spawn(&bus_pid, "/usr/local/bin/dbus-daemon", &fa, NULL, argv, environ);
  if (rc != 0) rc = posix_spawnp(&bus_pid, "dbus-daemon", &fa, NULL, argv, environ);
  posix_spawn_file_actions_destroy(&fa);
  if (rc != 0) { printf("  info cannot start dbus-daemon: %s\n", strerror(rc)); bus_pid = -1; return 0; }
  struct stat s;
  for (int i = 0; i < 100; ++i) {
    if (stat(sock, &s) == 0) { snprintf(addr, n, "unix:path=%s", sock); return 1; }
    int st;
    if (waitpid(bus_pid, &st, WNOHANG) == bus_pid) { bus_pid = -1; break; }
    usleep(100000);
  }
  return 0;
}

/* ---- the fake portal, in its own thread ---- */
static const char *XML =
  "<node><interface name='org.freedesktop.portal.Settings'>"
  " <method name='ReadOne'><arg type='s' direction='in'/><arg type='s' direction='in'/><arg type='v' direction='out'/></method>"
  " <method name='Read'><arg type='s' direction='in'/><arg type='s' direction='in'/><arg type='v' direction='out'/></method>"
  " <method name='ReadAll'><arg type='as' direction='in'/><arg type='a{sa{sv}}' direction='out'/></method>"
  " <signal name='SettingChanged'><arg type='s'/><arg type='s'/><arg type='v'/></signal>"
  " <property name='version' type='u' access='read'/>"
  "</interface></node>";
static GMainContext *pctx;
static GMainLoop *ploop;
static GDBusConnection *pconn;
static volatile int portal_ready, reads;
static GVariant *lookup(const char *ns, const char *key) {
  if (!strcmp(ns, "org.freedesktop.appearance") && !strcmp(key, "color-scheme")) return g_variant_new_uint32(1);
  if (!strcmp(ns, "org.qnx.test") && !strcmp(key, "greeting")) return g_variant_new_string("hello from the portal");
  return NULL;
}
static void method(GDBusConnection *c, const gchar *sender, const gchar *path, const gchar *iface, const gchar *name,
                   GVariant *params, GDBusMethodInvocation *inv, gpointer ud) {
  (void)c; (void)sender; (void)path; (void)iface; (void)ud;
  ++reads;
  if (!strcmp(name, "ReadOne") || !strcmp(name, "Read")) {
    const char *ns, *key;
    g_variant_get(params, "(&s&s)", &ns, &key);
    GVariant *v = lookup(ns, key);
    if (!v) { g_dbus_method_invocation_return_dbus_error(inv, "org.freedesktop.portal.Error.NotFound", "Requested setting not found"); return; }
    if (!strcmp(name, "Read")) v = g_variant_new_variant(v);
    g_dbus_method_invocation_return_value(inv, g_variant_new("(v)", v));
  } else {
    GVariantBuilder all, ns;
    g_variant_builder_init(&all, G_VARIANT_TYPE("a{sa{sv}}"));
    g_variant_builder_init(&ns, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&ns, "{sv}", "greeting", g_variant_new_string("hello from the portal"));
    g_variant_builder_add(&ns, "{sv}", "answer", g_variant_new_int32(42));
    g_variant_builder_add(&all, "{sa{sv}}", "org.qnx.test", &ns);
    g_dbus_method_invocation_return_value(inv, g_variant_new("(a{sa{sv}})", &all));
  }
}
static GVariant *getprop(GDBusConnection *c, const gchar *s, const gchar *p, const gchar *i, const gchar *n, GError **e, gpointer ud) {
  (void)c; (void)s; (void)p; (void)i; (void)n; (void)e; (void)ud;
  return g_variant_new_uint32(2);
}
static void name_acquired(GDBusConnection *c, const gchar *n, gpointer ud) { (void)c; (void)n; (void)ud; portal_ready = 1; }
static void name_lost(GDBusConnection *c, const gchar *n, gpointer ud) { (void)c; (void)n; (void)ud; portal_ready = -1; }
static gpointer portal_thread(gpointer addr) {
  g_main_context_push_thread_default(pctx);
  GError *e = NULL;
  pconn = g_dbus_connection_new_for_address_sync(addr, G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                                                 G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION, NULL, NULL, &e);
  if (!pconn) { printf("  info portal connection: %s\n", e->message); portal_ready = -1; return NULL; }
  GDBusNodeInfo *ni = g_dbus_node_info_new_for_xml(XML, NULL);
  static const GDBusInterfaceVTable vt = {method, getprop, NULL, {0}};
  g_dbus_connection_register_object(pconn, "/org/freedesktop/portal/desktop", ni->interfaces[0], &vt, NULL, NULL, NULL);
  g_bus_own_name_on_connection(pconn, "org.freedesktop.portal.Desktop", G_BUS_NAME_OWNER_FLAGS_NONE,
                               name_acquired, name_lost, NULL, NULL);
  g_main_loop_run(ploop);
  g_dbus_node_info_unref(ni);
  g_main_context_pop_thread_default(pctx);
  return NULL;
}
static gboolean emit_changed(gpointer ud) {
  (void)ud;
  g_dbus_connection_emit_signal(pconn, NULL, "/org/freedesktop/portal/desktop", "org.freedesktop.portal.Settings",
                                "SettingChanged", g_variant_new("(ssv)", "org.qnx.test", "greeting", g_variant_new_string("changed!")), NULL);
  return G_SOURCE_REMOVE;
}
static char changed_key[64], changed_val[64];
static void on_changed(XdpSettings *s, const char *ns, const char *key, GVariant *v, gpointer ud) {
  (void)s; (void)ns; (void)ud;
  snprintf(changed_key, sizeof changed_key, "%s", key);
  if (g_variant_is_of_type(v, G_VARIANT_TYPE_STRING)) snprintf(changed_val, sizeof changed_val, "%s", g_variant_get_string(v, NULL));
}

int main(void) {
  QT_BEGIN("libportal");
  alarm(90);
  QT_CHECK(!xdp_portal_running_under_flatpak() && !xdp_portal_running_under_sandbox(), "not running in a sandbox");
  char addr[200];
  int up = start_bus(addr, sizeof addr);   /* before any GLib thread exists */
  QT_CHECK(up, "private dbus-daemon started");
  if (!up) { stop_bus(); QT_END(); }
  setenv("DBUS_SESSION_BUS_ADDRESS", addr, 1);

  pctx = g_main_context_new();
  ploop = g_main_loop_new(pctx, FALSE);
  GThread *th = g_thread_new("fake-portal", portal_thread, addr);
  for (int i = 0; i < 100 && portal_ready == 0; ++i) usleep(50000);
  QT_CHECK(portal_ready == 1, "fake portal owns org.freedesktop.portal.Desktop");

  GError *e = NULL;
  XdpPortal *portal = portal_ready == 1 ? xdp_portal_initable_new(&e) : NULL;
  if (e) { printf("  info %s\n", e->message); g_clear_error(&e); }
  QT_CHECK(portal != NULL, "XdpPortal connected to the session bus");
  if (portal) {
    XdpSettings *st = xdp_portal_get_settings(portal);
    QT_CHECK(st != NULL, "settings object");
    char *g = xdp_settings_read_string(st, "org.qnx.test", "greeting", NULL, &e);
    if (e) { printf("  info %s\n", e->message); g_clear_error(&e); }
    QT_CHECK(g && strcmp(g, "hello from the portal") == 0, "read_string over D-Bus");
    g_free(g);
    guint cs = xdp_settings_read_uint(st, "org.freedesktop.appearance", "color-scheme", NULL, &e);
    if (e) { printf("  info %s\n", e->message); g_clear_error(&e); }
    QT_CHECK(cs == 1, "read_uint: color-scheme = 1 (prefer dark)");
    GVariant *v = xdp_settings_read_value(st, "org.qnx.test", "missing", NULL, &e);
    QT_CHECK(v == NULL && e != NULL, "unknown key returns a NotFound error");
    if (e) { printf("  info error: %s\n", e->message); g_clear_error(&e); }
    const char *const nss[] = {"org.qnx.test", NULL};
    GVariant *all = xdp_settings_read_all_values(st, nss, NULL, &e);
    if (e) { printf("  info %s\n", e->message); g_clear_error(&e); }
    gint32 ans = 0;
    if (all) {
      GVariant *nsd = g_variant_lookup_value(all, "org.qnx.test", G_VARIANT_TYPE("a{sv}"));
      if (nsd) { g_variant_lookup(nsd, "answer", "i", &ans); g_variant_unref(nsd); }
      g_variant_unref(all);
    }
    QT_CHECK(ans == 42, "read_all_values returns the namespace dictionary");

    g_signal_connect(st, "changed", G_CALLBACK(on_changed), NULL);
    g_main_context_invoke(pctx, emit_changed, NULL);
    for (int i = 0; i < 100 && !changed_key[0]; ++i) {
      while (g_main_context_iteration(NULL, FALSE)) {}
      usleep(50000);
    }
    QT_CHECK(strcmp(changed_key, "greeting") == 0 && strcmp(changed_val, "changed!") == 0, "SettingChanged delivered as \"changed\"");
    printf("  info portal served %d call(s)\n", (int)reads);
    g_object_unref(st);
    g_object_unref(portal);
  }
  g_main_loop_quit(ploop);
  g_thread_join(th);
  if (pconn) g_object_unref(pconn);
  stop_bus();
  char p[160];
  snprintf(p, sizeof p, "%s/bus.conf", DIR_); unlink(p);
  snprintf(p, sizeof p, "%s/dbus.log", DIR_); unlink(p);
  snprintf(p, sizeof p, "%s/bus", DIR_); unlink(p);
  rmdir(DIR_);
  QT_END();
}
