/* libxfce4windowing on Wayland: start labwc on wlroots' headless backend,
 * open GTK on it, and check that libxfce4windowing detects Wayland, gives a
 * screen with a seat, and -- through labwc's wlr-foreign-toplevel protocol --
 * lists a GTK window this test maps, with the title it was given; then the
 * title is changed and the change must be seen as well. */
#include "common.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <libxfce4windowing/libxfce4windowing.h>
#include <fcntl.h>
/* ---- cleanup guard: every process this test starts is stopped on every
 * exit path -- return from main, exit(), a fatal signal (a crash in the
 * library under test) and the watchdog alarm. Each child leads a process
 * group of its own, so what it starts in turn (labwc may launch Xwayland or
 * helpers) is stopped with it. Left running, labwc keeps libdrm/libgbm busy
 * and breaks whatever runs on the target next. ---- */
#define QX_MAX_KIDS 8
static pid_t qx_kids[QX_MAX_KIDS];
static int qx_nkids;
static void qx_stop(pid_t p) {
  if (p <= 0) return;
  kill(-p, SIGTERM); kill(p, SIGTERM);
  int done = 0;
  for (int t = 0; t < 30 && !done; t++) { if (waitpid(p, NULL, WNOHANG) != 0) done = 1; else usleep(100000); }
  if (!done) { kill(-p, SIGKILL); kill(p, SIGKILL); waitpid(p, NULL, 0); }
  kill(-p, SIGKILL);   /* anything left in its group */
  for (int i = 0; i < qx_nkids; i++) if (qx_kids[i] == p) qx_kids[i] = -1;
}
static void qx_stop_all(void) {
  for (int i = qx_nkids - 1; i >= 0; i--) if (qx_kids[i] > 0) qx_stop(qx_kids[i]);
}
static void qx_on_signal(int sig) {
  static const char m[] = "  FAIL watchdog: test did not finish in time\n";
  if (sig == SIGALRM) (void)!write(1, m, sizeof m - 1);
  qx_stop_all();
  signal(sig, SIG_DFL); raise(sig);
}
static void qx_guard(unsigned seconds) {
  static int armed;
  if (armed) return;
  armed = 1;
  atexit(qx_stop_all);
  const int sigs[] = { SIGINT, SIGTERM, SIGHUP, SIGQUIT, SIGSEGV, SIGBUS, SIGABRT, SIGFPE, SIGILL, SIGALRM };
  for (unsigned i = 0; i < sizeof sigs / sizeof *sigs; i++) signal(sigs[i], qx_on_signal);
  alarm(seconds);
}
/* fork + exec in a new process group, recorded for qx_stop_all; log == NULL
   keeps the test's stdout/stderr */
static pid_t qx_spawn(char *const a[], const char *log) {
  qx_guard(240);
  pid_t p = fork();
  if (p == 0) {
    setpgid(0, 0);
    if (log) { int fd = open(log, O_WRONLY | O_CREAT | O_APPEND, 0644); if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); close(fd); } }
    execvp(a[0], a); _exit(127);
  }
  if (p > 0) { setpgid(p, p); if (qx_nkids < QX_MAX_KIDS) qx_kids[qx_nkids++] = p; }
  return p;
}
static pid_t spawn(char *const a[]) { return qx_spawn(a, NULL); }
static XfwWindow *find_win(XfwScreen *s, const char *title) {
  for (GList *l = xfw_screen_get_windows(s); l; l = l->next) {
    const gchar *n = xfw_window_get_name(XFW_WINDOW(l->data));
    if (n && strcmp(n, title) == 0) return XFW_WINDOW(l->data);
  }
  return NULL;
}
/* run the GLib main loop until the window shows up (bounded: 200 x 50 ms) */
static XfwWindow *wait_win(XfwScreen *s, const char *title) {
  XfwWindow *w = NULL;
  for (int i = 0; i < 200 && !(w = find_win(s, title)); i++) {
    while (g_main_context_iteration(NULL, FALSE)) ;
    usleep(50000);
  }
  return w;
}
int main(int argc, char **argv) {
  QT_BEGIN("libxfce4windowing");
  char rt[] = "/tmp/qnxport-xfw-XXXXXX";
  QT_CHECK(mkdtemp(rt) != NULL, "private XDG_RUNTIME_DIR");
  chmod(rt, 0700);
  setenv("XDG_RUNTIME_DIR", rt, 1); setenv("WLR_BACKENDS", "headless", 1); setenv("WLR_RENDERER", "pixman", 1);
  unsetenv("WAYLAND_DISPLAY"); unsetenv("DISPLAY");
  char *lab[] = { "labwc", NULL };
  pid_t lp = spawn(lab);
  char sock[300]; snprintf(sock, sizeof sock, "%s/wayland-0", rt);
  struct stat sb; int up = 0;
  for (int i = 0; i < 150 && !up; i++) { if (stat(sock, &sb) == 0) up = 1; else usleep(100000); }
  QT_CHECK(up, "labwc (headless) Wayland socket appeared");
  if (up) {
    setenv("WAYLAND_DISPLAY", "wayland-0", 1); setenv("GDK_BACKEND", "wayland", 1);
    int gtk_ok = gtk_init_check(&argc, &argv);
    QT_CHECK(gtk_ok, "gtk_init_check on labwc");
    if (gtk_ok) {
      XfwWindowing wg = xfw_windowing_get();
      printf("  info xfw_windowing_get = %d (2 = Wayland)\n", (int)wg);
      QT_CHECK(wg == XFW_WINDOWING_WAYLAND, "windowing detected as Wayland");
      XfwScreen *s = xfw_screen_get_default();
      QT_CHECK(s != NULL, "xfw_screen_get_default");
      if (s) {
        GtkWidget *gw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(gw), "qnxport-xfw-one");
        gtk_window_set_default_size(GTK_WINDOW(gw), 120, 80);
        gtk_widget_show_all(gw);
        XfwWindow *w = wait_win(s, "qnxport-xfw-one");
        QT_CHECK(w != NULL, "mapped GTK window listed by xfw_screen_get_windows");
        printf("  info %u window(s) listed\n", g_list_length(xfw_screen_get_windows(s)));
        if (w) {
          QT_CHECK(xfw_window_get_screen(w) == s, "window belongs to the screen");
          gtk_window_set_title(GTK_WINDOW(gw), "qnxport-xfw-two");
          QT_CHECK(wait_win(s, "qnxport-xfw-two") != NULL, "title change seen through the compositor");
        }
        GList *seats = xfw_screen_get_seats(s);
        printf("  info %u seat(s)%s%s\n", g_list_length(seats), seats ? ", first: " : "",
               seats && xfw_seat_get_name(seats->data) ? xfw_seat_get_name(seats->data) : "");
        QT_CHECK(seats != NULL, "screen has a seat");
        GList *mons = xfw_screen_get_monitors(s);
        printf("  info %u monitor(s)\n", g_list_length(mons));
        XfwWorkspaceManager *wm = xfw_screen_get_workspace_manager(s);
        if (wm) printf("  info %u workspace(s)\n", g_list_length(xfw_workspace_manager_list_workspaces(wm)));
        gtk_widget_destroy(gw);
        while (g_main_context_iteration(NULL, FALSE)) ;
        g_object_unref(s);
      }
    }
  }
  qx_stop(lp);
  unlink(sock); char lk[310]; snprintf(lk, sizeof lk, "%s.lock", sock); unlink(lk); rmdir(rt);
  QT_END();
}
