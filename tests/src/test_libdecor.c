/* libdecor: against a headless Wayland compositor (labwc, as in test_libxft)
 * -- connect, create a libdecor context (it loads its plugin: cairo or GTK,
 * or none when the compositor draws server-side decorations), decorate a
 * wl_surface, set title and app id, map the frame and dispatch until the
 * compositor's first configure arrives; the frame reports the title back,
 * its capabilities, and commit a content size. */
#include "common.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <wayland-client.h>
#include <libdecor.h>

#define QX_MAX_KIDS 4
static pid_t qx_kids[QX_MAX_KIDS];
static int qx_nkids;
static void qx_stop(pid_t p) {
  if (p <= 0) return;
  kill(-p, SIGTERM); kill(p, SIGTERM);
  int done = 0;
  for (int t = 0; t < 30 && !done; t++) { if (waitpid(p, NULL, WNOHANG) != 0) done = 1; else usleep(100000); }
  if (!done) { kill(-p, SIGKILL); kill(p, SIGKILL); waitpid(p, NULL, 0); }
  kill(-p, SIGKILL);
  for (int i = 0; i < qx_nkids; i++) if (qx_kids[i] == p) qx_kids[i] = -1;
}
static void qx_stop_all(void) { for (int i = qx_nkids - 1; i >= 0; i--) if (qx_kids[i] > 0) qx_stop(qx_kids[i]); }
static void qx_on_signal(int sig) {
  static const char m[] = "  FAIL watchdog: test did not finish in time\n";
  if (sig == SIGALRM) (void)!write(1, m, sizeof m - 1);
  qx_stop_all(); signal(sig, SIG_DFL); raise(sig);
}
static pid_t qx_spawn(char *const a[], const char *log) {
  static int armed;
  if (!armed) {
    armed = 1; atexit(qx_stop_all);
    const int sigs[] = { SIGINT, SIGTERM, SIGHUP, SIGQUIT, SIGSEGV, SIGBUS, SIGABRT, SIGFPE, SIGILL, SIGALRM };
    for (unsigned i = 0; i < sizeof sigs / sizeof *sigs; i++) signal(sigs[i], qx_on_signal);
    alarm(120);
  }
  pid_t p = fork();
  if (p == 0) {
    setpgid(0, 0);
    if (log) { int fd = open(log, O_WRONLY | O_CREAT | O_APPEND, 0644); if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); close(fd); } }
    execvp(a[0], a); _exit(127);
  }
  if (p > 0) { setpgid(p, p); if (qx_nkids < QX_MAX_KIDS) qx_kids[qx_nkids++] = p; }
  return p;
}
static int wait_file(const char *p, int tenths) { struct stat s; while (tenths-- > 0) { if (stat(p, &s) == 0) return 1; usleep(100000); } return 0; }

static struct wl_compositor *comp;
static void reg_global(void *d, struct wl_registry *r, uint32_t name, const char *iface, uint32_t v) {
  (void)d; (void)v;
  if (!strcmp(iface, wl_compositor_interface.name)) comp = wl_registry_bind(r, name, &wl_compositor_interface, 4);
}
static void reg_remove(void *d, struct wl_registry *r, uint32_t n) { (void)d; (void)r; (void)n; }
static const struct wl_registry_listener reg_l = { reg_global, reg_remove };

static int configured, conf_w, conf_h, errors;
static void on_error(struct libdecor *ctx, enum libdecor_error e, const char *msg) { (void)ctx; (void)e; printf("  info libdecor error: %s\n", msg); errors++; }
static struct libdecor_interface iface = { .error = on_error };
static void on_configure(struct libdecor_frame *fr, struct libdecor_configuration *cf, void *ud) {
  (void)ud;
  int w = 0, h = 0;
  if (!libdecor_configuration_get_content_size(cf, fr, &w, &h)) { w = 320; h = 240; }
  conf_w = w; conf_h = h;
  struct libdecor_state *st = libdecor_state_new(w, h);
  libdecor_frame_commit(fr, st, cf);
  libdecor_state_free(st);
  configured++;
}
static void on_close(struct libdecor_frame *fr, void *ud) { (void)fr; (void)ud; }
static void on_commit(struct libdecor_frame *fr, void *ud) { (void)fr; (void)ud; }
static struct libdecor_frame_interface frame_iface = { .configure = on_configure, .close = on_close, .commit = on_commit };

int main(void) {
  QT_BEGIN("libdecor");
  char rt[] = "/tmp/qnxport-decor-XXXXXX";
  if (!mkdtemp(rt)) { QT_CHECK(0, "runtime dir"); QT_END(); }
  chmod(rt, 0700);
  setenv("XDG_RUNTIME_DIR", rt, 1); setenv("WLR_BACKENDS", "headless", 1); setenv("WLR_RENDERER", "pixman", 1);
  unsetenv("WAYLAND_DISPLAY"); unsetenv("DISPLAY");
  char log[300]; snprintf(log, sizeof log, "%s/labwc.log", rt);
  char *lab[] = { "labwc", NULL }; qx_spawn(lab, log);
  char ws[300]; snprintf(ws, sizeof ws, "%s/wayland-0", rt);
  QT_CHECK(wait_file(ws, 150), "labwc up (headless Wayland compositor)");
  setenv("WAYLAND_DISPLAY", "wayland-0", 1);
  struct wl_display *d = NULL;
  for (int i = 0; i < 50 && !d; i++) { d = wl_display_connect(NULL); if (!d) usleep(100000); }
  QT_CHECK(d != NULL, "wl_display_connect");
  if (!d) { qx_stop_all(); QT_END(); }
  struct wl_registry *reg = wl_display_get_registry(d);
  wl_registry_add_listener(reg, &reg_l, NULL);
  wl_display_roundtrip(d);
  QT_CHECK(comp != NULL, "wl_compositor bound");
  struct libdecor *ctx = libdecor_new(d, &iface);
  QT_CHECK(ctx != NULL, "libdecor_new (plugin loaded)");
  struct wl_surface *s = comp ? wl_compositor_create_surface(comp) : NULL;
  struct libdecor_frame *fr = (ctx && s) ? libdecor_decorate(ctx, s, &frame_iface, NULL) : NULL;
  QT_CHECK(fr != NULL, "libdecor_decorate a wl_surface");
  if (fr) {
    libdecor_frame_set_title(fr, "QNX libdecor");
    libdecor_frame_set_app_id(fr, "org.openqnx.test");
    libdecor_frame_map(fr);
    for (int i = 0; i < 100 && !configured; i++) libdecor_dispatch(ctx, 50);
    printf("  info configured %d time(s), content %dx%d\n", configured, conf_w, conf_h);
    QT_CHECK(configured >= 1 && conf_w > 0 && conf_h > 0, "the compositor's configure arrives and is committed");
    QT_CHECK(!strcmp(libdecor_frame_get_title(fr), "QNX libdecor"), "the frame keeps its title");
    QT_CHECK(libdecor_frame_has_capability(fr, LIBDECOR_ACTION_CLOSE), "the frame can be closed");
    libdecor_frame_unref(fr);
  }
  QT_CHECK(errors == 0, "no libdecor errors");
  if (ctx) libdecor_unref(ctx);
  wl_display_disconnect(d);
  qx_stop_all();
  char cmd[340]; snprintf(cmd, sizeof cmd, "rm -rf %s", rt); if (system(cmd)) {}
  QT_END();
}
