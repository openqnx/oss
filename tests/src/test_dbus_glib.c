/* dbus-glib: a private dbus-daemon started here, used through the GLib
 * bindings. Checks: dbus_g_bus_get() connects; a DBusGProxy for the bus
 * calls GetId (32 hex digits), RequestName (org.qnx.DbusGlibTest ->
 * PRIMARY_OWNER, 1), NameHasOwner, GetNameOwner (our unique name),
 * ListNames (contains the name, as a G_TYPE_STRV); an async call
 * (dbus_g_proxy_begin_call/end_call) completes in the GLib main loop; the
 * NameOwnerChanged signal, registered with dbus_g_proxy_add_signal, is
 * delivered when the name is released; a call to a missing method fails
 * with DBUS_GERROR_UNKNOWN_METHOD; and
 * dbus-binding-tool generates a client stub for an interface. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>   /* dbus_g_connection_get_connection */
#include <dbus/dbus.h>

#define NAME "org.qnx.DbusGlibTest"
static pid_t bus = -1; static char dir[64];
static void stop(void) { if (bus > 0) { kill(bus, SIGTERM); waitpid(bus, NULL, 0); bus = -1; } }
static void on_alarm(int s) { (void)s; if (bus > 0) kill(bus, SIGKILL); static const char m[] = "FAIL dbus-glib (watchdog)\n"; (void)!write(1, m, sizeof m - 1); _exit(2); }
static GMainLoop *loop;
static char *changed_name, *changed_old; static int changed;
static void on_owner_changed(DBusGProxy *p, const char *name, const char *old, const char *nw, gpointer d) {
  (void)p; (void)d;
  if (!strcmp(name, NAME) && !*nw) { changed = 1; changed_name = g_strdup(name); changed_old = g_strdup(old); g_main_loop_quit(loop); }
}
static char *async_id; static int async_done;
static void on_getid(DBusGProxy *p, DBusGProxyCall *call, gpointer d) {
  (void)d; GError *e = NULL;
  if (dbus_g_proxy_end_call(p, call, &e, G_TYPE_STRING, &async_id, G_TYPE_INVALID)) async_done = 1;
  else g_clear_error(&e);
  g_main_loop_quit(loop);
}
static gboolean timeout_quit(gpointer d) { (void)d; g_main_loop_quit(loop); return FALSE; }

int main(void) {
  QT_BEGIN("dbus-glib");
  signal(SIGALRM, on_alarm); alarm(90); atexit(stop);
  snprintf(dir, sizeof dir, "/tmp/qnxport-dbusglib-%d", (int)getpid()); mkdir(dir, 0700);
  char sock[128], conf[128], carg[160];
  snprintf(sock, sizeof sock, "%s/bus", dir); snprintf(conf, sizeof conf, "%s/bus.conf", dir);
  FILE *f = fopen(conf, "w");
  if (f) { fprintf(f, "<!DOCTYPE busconfig PUBLIC \"-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN\"\n"
    " \"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">\n<busconfig>\n  <type>session</type>\n"
    "  <listen>unix:path=%s</listen>\n  <allow_anonymous/>\n  <policy context=\"default\">\n"
    "    <allow send_destination=\"*\" eavesdrop=\"true\"/>\n    <allow eavesdrop=\"true\"/>\n    <allow own=\"*\"/>\n"
    "  </policy>\n</busconfig>\n", sock); fclose(f); }
  snprintf(carg, sizeof carg, "--config-file=%s", conf);
  bus = fork();
  if (bus == 0) { execl("/usr/local/bin/dbus-daemon", "dbus-daemon", carg, "--nofork", "--nopidfile", (char *)NULL);
                  execlp("dbus-daemon", "dbus-daemon", carg, "--nofork", "--nopidfile", (char *)NULL); _exit(127); }
  struct stat st; int up = 0;
  for (int i = 0; i < 100 && !up; i++) { if (stat(sock, &st) == 0) up = 1; else usleep(100000); }
  QT_CHECK(up, "private dbus-daemon listening on a UNIX socket");
  char addr[160]; snprintf(addr, sizeof addr, "unix:path=%s", sock);
  setenv("DBUS_SESSION_BUS_ADDRESS", addr, 1);

  GError *err = NULL;
  DBusGConnection *conn = up ? dbus_g_bus_get(DBUS_BUS_SESSION, &err) : NULL;
  QT_CHECK(conn != NULL, "dbus_g_bus_get(DBUS_BUS_SESSION)");
  if (!conn) { if (err) printf("  info %s\n", err->message); QT_END(); }
  loop = g_main_loop_new(NULL, FALSE);
  DBusGProxy *bp = dbus_g_proxy_new_for_name(conn, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

  char *id = NULL;
  int ok = dbus_g_proxy_call(bp, "GetId", &err, G_TYPE_INVALID, G_TYPE_STRING, &id, G_TYPE_INVALID);
  int hex = ok && id && strlen(id) == 32 && strspn(id, "0123456789abcdef") == 32;
  QT_CHECK(hex, "GetId: a 32-hex-digit bus id");
  guint rn = 0;
  ok = dbus_g_proxy_call(bp, "RequestName", &err, G_TYPE_STRING, NAME, G_TYPE_UINT, 0, G_TYPE_INVALID,
                         G_TYPE_UINT, &rn, G_TYPE_INVALID);
  QT_CHECK(ok && rn == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER, "RequestName(" NAME ") -> PRIMARY_OWNER (1)");
  gboolean has = FALSE;
  ok = dbus_g_proxy_call(bp, "NameHasOwner", &err, G_TYPE_STRING, NAME, G_TYPE_INVALID, G_TYPE_BOOLEAN, &has, G_TYPE_INVALID);
  QT_CHECK(ok && has, "NameHasOwner: TRUE");
  char *owner = NULL;
  ok = dbus_g_proxy_call(bp, "GetNameOwner", &err, G_TYPE_STRING, NAME, G_TYPE_INVALID, G_TYPE_STRING, &owner, G_TYPE_INVALID);
  const char *uniq = dbus_bus_get_unique_name(dbus_g_connection_get_connection(conn));
  QT_CHECK(ok && owner && uniq && !strcmp(owner, uniq), "GetNameOwner: our own unique name");
  printf("  info unique name %s\n", uniq ? uniq : "?");
  char **names = NULL; int found = 0;
  ok = dbus_g_proxy_call(bp, "ListNames", &err, G_TYPE_INVALID, G_TYPE_STRV, &names, G_TYPE_INVALID);
  for (char **n = names; ok && n && *n; n++) if (!strcmp(*n, NAME)) found = 1;
  QT_CHECK(found, "ListNames (G_TYPE_STRV) contains " NAME);
  g_strfreev(names);

  /* async call through the main loop */
  dbus_g_proxy_begin_call(bp, "GetId", on_getid, NULL, NULL, G_TYPE_INVALID);
  guint t = g_timeout_add(10000, timeout_quit, NULL);
  g_main_loop_run(loop); g_source_remove(t);
  QT_CHECK(async_done && id && async_id && !strcmp(async_id, id), "begin_call/end_call in the main loop: the same bus id");

  /* signals */
  dbus_g_proxy_add_signal(bp, "NameOwnerChanged", G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
  dbus_g_proxy_connect_signal(bp, "NameOwnerChanged", G_CALLBACK(on_owner_changed), NULL, NULL);
  guint rel = 0;
  dbus_g_proxy_call(bp, "ReleaseName", &err, G_TYPE_STRING, NAME, G_TYPE_INVALID, G_TYPE_UINT, &rel, G_TYPE_INVALID);
  t = g_timeout_add(10000, timeout_quit, NULL);
  if (!changed) g_main_loop_run(loop);
  g_source_remove(t);
  QT_CHECK(rel == DBUS_RELEASE_NAME_REPLY_RELEASED && changed && uniq && !strcmp(changed_old, uniq),
           "ReleaseName -> NameOwnerChanged(" NAME ", us, \"\") delivered to the signal handler");

  /* errors */
  g_clear_error(&err);
  ok = dbus_g_proxy_call(bp, "NoSuchMethod", &err, G_TYPE_INVALID, G_TYPE_INVALID);
  /* a well-known D-Bus error name is mapped to its own DBUS_GERROR code */
  QT_CHECK(!ok && err && err->domain == DBUS_GERROR && err->code == DBUS_GERROR_UNKNOWN_METHOD,
           "a missing method: GError DBUS_GERROR_UNKNOWN_METHOD");
  if (err) printf("  info error %d: %s\n", err->code, err->message);
  g_clear_error(&err);
  g_object_unref(bp);
  dbus_g_connection_unref(conn);

  /* the code generator */
  char xml[128]; snprintf(xml, sizeof xml, "%s/iface.xml", dir);
  f = fopen(xml, "w");
  if (f) { fputs("<node name=\"/org/qnx/Test\"><interface name=\"org.qnx.Test\">"
                 "<method name=\"Echo\"><arg type=\"s\" name=\"text\" direction=\"in\"/>"
                 "<arg type=\"s\" name=\"reply\" direction=\"out\"/></method></interface></node>\n", f); fclose(f); }
  char cmd[256]; snprintf(cmd, sizeof cmd, "dbus-binding-tool --mode=glib-client --prefix=qnx %s 2>&1", xml);
  FILE *p = popen(cmd, "r"); static char gen[16384]; size_t gn = p ? fread(gen, 1, sizeof gen - 1, p) : 0; gen[gn] = 0; if (p) pclose(p);
  QT_CHECK(strstr(gen, "org_qnx_Test_echo") && strstr(gen, "dbus_g_proxy_call") && strstr(gen, "IN_text") && strstr(gen, "OUT_reply"),
           "dbus-binding-tool --mode=glib-client: a stub org_qnx_Test_echo(proxy, IN_text, OUT_reply, ...)");
  stop();
  snprintf(cmd, sizeof cmd, "rm -rf %s", dir); (void)!system(cmd);
  QT_END();
}
