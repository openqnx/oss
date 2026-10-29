#include "common.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wayland-client.h>
/* labwc as a real compositor, on wlroots' headless backend (no display
 * needed): started, its Wayland socket appears, a client connects and finds
 * the globals a desktop needs (compositor, shm, seat, xdg-shell for windows,
 * wlr-layer-shell for panels and backgrounds, QNX's qnx-redir only on the
 * Screen backend). Then swaybg, a layer-shell client, sets a background and
 * must stay running; labwc must outlive it and exit on SIGTERM. */
static const char *want[] = { "wl_compositor", "wl_shm", "wl_seat", "xdg_wm_base",
                              "zwlr_layer_shell_v1", "wl_data_device_manager" };
static int seen[6];
static void global(void *d, struct wl_registry *r, uint32_t id, const char *iface, uint32_t v) {
  (void)d; (void)r; (void)id; (void)v;
  for (int i = 0; i < 6; i++) if (strcmp(iface, want[i]) == 0) seen[i] = 1;
}
static void global_remove(void *d, struct wl_registry *r, uint32_t id) { (void)d; (void)r; (void)id; }
static const struct wl_registry_listener rl = { global, global_remove };

static pid_t spawn(char *const argv[]) {
  pid_t p = fork();
  if (p == 0) { execvp(argv[0], argv); _exit(127); }
  return p;
}

int main(void) {
  QT_BEGIN("labwc");
  char rt[] = "/tmp/labwc-rt-XXXXXX";
  QT_CHECK(mkdtemp(rt) != NULL, "XDG_RUNTIME_DIR");
  chmod(rt, 0700);
  setenv("XDG_RUNTIME_DIR", rt, 1);
  setenv("WLR_BACKENDS", "headless", 1);
  setenv("WLR_RENDERER", "pixman", 1);
  unsetenv("WAYLAND_DISPLAY");

  char *lab[] = { "labwc", NULL };
  pid_t lp = spawn(lab);
  QT_CHECK(lp > 0, "labwc started");
  char sock[300]; snprintf(sock, sizeof sock, "%s/wayland-0", rt);
  struct stat sb; int up = 0;
  for (int i = 0; i < 150 && !up; i++) { if (stat(sock, &sb) == 0) up = 1; else usleep(100000); }
  QT_CHECK(up, "labwc's Wayland socket wayland-0 appeared");
  int st;
  if (!up && waitpid(lp, &st, WNOHANG) == lp)
    printf("  info labwc exited early, status %d\n", WIFEXITED(st) ? WEXITSTATUS(st) : -WTERMSIG(st));

  if (up) {
    setenv("WAYLAND_DISPLAY", "wayland-0", 1);
    struct wl_display *d = wl_display_connect(NULL);
    QT_CHECK(d != NULL, "client connected to labwc");
    if (d) {
      struct wl_registry *reg = wl_display_get_registry(d);
      wl_registry_add_listener(reg, &rl, NULL);
      wl_display_roundtrip(d);
      for (int i = 0; i < 6; i++) {
        char what[96]; snprintf(what, sizeof what, "global %s", want[i]);
        QT_CHECK(seen[i], what);
      }
      wl_registry_destroy(reg);
      wl_display_disconnect(d);
    }
    if (access("/usr/local/bin/swaybg", X_OK) == 0 || access("/data/oss/usr/local/bin/swaybg", X_OK) == 0) {
      char *bg[] = { "swaybg", "-c", "#336699", "-m", "solid_color", NULL };
      pid_t bp = spawn(bg);
      sleep(2);
      QT_CHECK(waitpid(bp, &st, WNOHANG) == 0, "swaybg running on labwc (background layer surface)");
      kill(bp, SIGTERM); waitpid(bp, &st, 0);
    } else printf("  info swaybg not installed -- background client not checked\n");
    QT_CHECK(waitpid(lp, &st, WNOHANG) == 0, "labwc still running after its clients");
  }
  if (lp > 0) {
    kill(lp, SIGTERM);
    int ok = 0;
    for (int i = 0; i < 50 && !ok; i++) { if (waitpid(lp, &st, WNOHANG) == lp) ok = 1; else usleep(100000); }
    if (!ok) { kill(lp, SIGKILL); waitpid(lp, &st, 0); }
    QT_CHECK(ok, "labwc exits on SIGTERM");
  }
  unlink(sock); char lk[310]; snprintf(lk, sizeof lk, "%s.lock", sock); unlink(lk); rmdir(rt);
  QT_END();
}
