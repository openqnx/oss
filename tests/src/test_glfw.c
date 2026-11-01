/* GLFW, headless: first the null platform -- its monitor "Null SuperNoop 0"
 * (1920x1080, 345x194 mm at 141 dpi), a window without a client API
 * (size, position, title, framebuffer size, the size callback), the
 * clipboard, key names, the timer -- then the Wayland platform against
 * labwc on wlroots' headless backend: glfwInit picks Wayland, finds the
 * compositor's output, and a GLFW_NO_API window is created, retitled,
 * resized and destroyed while events are pumped. GLFW dlopens the Wayland
 * and xkbcommon libraries (n3-gfx-py.extras). */
#include "common.h"
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
/* no OpenGL headers on QNX (EGL/GLES only): the window API without GL */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

/* ---- every process this test starts is stopped on every exit path ---- */
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
    alarm(150);
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

static char last_error[512];
static void on_error(int code, const char *desc) {
  snprintf(last_error, sizeof last_error, "0x%x %s", code, desc ? desc : "");
  printf("  info GLFW error %s\n", last_error);
}
static int cb_w, cb_h;
static void on_size(GLFWwindow *w, int width, int height) { (void)w; cb_w = width; cb_h = height; }
static void sleep_ms(int ms) { struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L }; nanosleep(&ts, NULL); }

static void null_platform(void) {
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_NULL);
  int ok = glfwInit();
  QT_CHECK(ok, "glfwInit with GLFW_PLATFORM_NULL");
  if (!ok) return;
  QT_CHECK(glfwGetPlatform() == GLFW_PLATFORM_NULL, "glfwGetPlatform() = GLFW_PLATFORM_NULL");
  int count = 0;
  GLFWmonitor **mons = glfwGetMonitors(&count);
  QT_CHECK(count == 1 && mons && strcmp(glfwGetMonitorName(mons[0]), "Null SuperNoop 0") == 0, "one monitor, \"Null SuperNoop 0\"");
  if (count == 1) {
    const GLFWvidmode *vm = glfwGetVideoMode(mons[0]);
    QT_CHECK(vm && vm->width == 1920 && vm->height == 1080 && vm->refreshRate == 60, "video mode 1920x1080 @ 60 Hz");
    int mw = 0, mh = 0; glfwGetMonitorPhysicalSize(mons[0], &mw, &mh);
    QT_CHECK(mw == 345 && mh == 194, "physical size 345 x 194 mm (141 dpi)");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *w = glfwCreateWindow(320, 240, "qnx-glfw-null", NULL, NULL);
  QT_CHECK(w != NULL, "glfwCreateWindow 320x240 (GLFW_NO_API)");
  if (w) {
    int x, y, fw, fh;
    glfwGetWindowSize(w, &x, &y);
    QT_CHECK(x == 320 && y == 240, "glfwGetWindowSize = 320x240");
    glfwSetWindowSizeCallback(w, on_size);
    glfwSetWindowSize(w, 400, 300);
    glfwPollEvents();
    glfwGetWindowSize(w, &x, &y); glfwGetFramebufferSize(w, &fw, &fh);
    QT_CHECK(x == 400 && y == 300 && fw == 400 && fh == 300, "resized to 400x300, framebuffer too");
    QT_CHECK(cb_w == 400 && cb_h == 300, "size callback reported 400x300");
    glfwSetWindowPos(w, 17, 23); glfwGetWindowPos(w, &x, &y);
    QT_CHECK(x == 17 && y == 23, "window position 17,23");
    glfwSetWindowTitle(w, "renamed");
    QT_CHECK(strcmp(glfwGetWindowTitle(w), "renamed") == 0, "window title set and read back");
    QT_CHECK(!glfwWindowShouldClose(w), "window not asked to close");
    glfwSetWindowShouldClose(w, GLFW_TRUE);
    QT_CHECK(glfwWindowShouldClose(w), "glfwSetWindowShouldClose");
    glfwSetClipboardString(w, "qnx clipboard");
    const char *cs = glfwGetClipboardString(w);
    QT_CHECK(cs && strcmp(cs, "qnx clipboard") == 0, "clipboard round trip");
    glfwDestroyWindow(w);
  }
  const char *kn = glfwGetKeyName(GLFW_KEY_SLASH, 0), *kc = glfwGetKeyName(GLFW_KEY_COMMA, 0);
  QT_CHECK(kn && kc && strcmp(kn, "/") == 0 && strcmp(kc, ",") == 0, "key names: / and ,");
  glfwSetTime(10.0); sleep_ms(50);
  double t = glfwGetTime();
  QT_CHECK(t >= 10.04 && t < 12.0, "glfwSetTime(10) + 50 ms sleep");
  glfwTerminate();
}

static void wayland_platform(void) {
  char rt[] = "/tmp/qnxport-glfw-XXXXXX";
  if (!mkdtemp(rt)) { QT_CHECK(0, "private XDG_RUNTIME_DIR"); return; }
  chmod(rt, 0700);
  setenv("XDG_RUNTIME_DIR", rt, 1); setenv("WLR_BACKENDS", "headless", 1); setenv("WLR_RENDERER", "pixman", 1);
  unsetenv("WAYLAND_DISPLAY"); unsetenv("DISPLAY");
  char *lab[] = { "labwc", NULL };
  char lg[300]; snprintf(lg, sizeof lg, "%s/labwc.log", rt);
  pid_t lp = qx_spawn(lab, lg);
  char sock[300]; snprintf(sock, sizeof sock, "%s/wayland-0", rt);
  struct stat sb; int up = 0;
  for (int i = 0; i < 150 && !up; i++) { if (stat(sock, &sb) == 0) up = 1; else usleep(100000); }
  QT_CHECK(up, "labwc (headless) Wayland socket appeared");
  if (up) {
    setenv("WAYLAND_DISPLAY", "wayland-0", 1);
    QT_CHECK(glfwPlatformSupported(GLFW_PLATFORM_WAYLAND), "glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)");
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    int ok = glfwInit();
    QT_CHECK(ok, "glfwInit on Wayland (libwayland-client, libxkbcommon loaded)");
    if (ok) {
      QT_CHECK(glfwGetPlatform() == GLFW_PLATFORM_WAYLAND, "glfwGetPlatform() = GLFW_PLATFORM_WAYLAND");
      int count = 0;
      GLFWmonitor **mons = glfwGetMonitors(&count);
      for (int i = 0; i < count; i++) {
        const GLFWvidmode *vm = glfwGetVideoMode(mons[i]);
        printf("  info monitor %d: %s %dx%d\n", i, glfwGetMonitorName(mons[i]), vm ? vm->width : 0, vm ? vm->height : 0);
      }
      QT_CHECK(count >= 1, "labwc's headless output seen as a monitor");
      glfwDefaultWindowHints();
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      GLFWwindow *w = glfwCreateWindow(320, 240, "qnx-glfw-wayland", NULL, NULL);
      QT_CHECK(w != NULL, "Wayland window created (xdg-shell, GLFW_NO_API)");
      if (w) {
        for (int i = 0; i < 20; i++) { glfwPollEvents(); sleep_ms(20); }
        int x = 0, y = 0; glfwGetWindowSize(w, &x, &y);
        printf("  info window size %dx%d\n", x, y);
        QT_CHECK(x > 0 && y > 0, "window has a size");
        glfwSetWindowTitle(w, "qnx-glfw-renamed");
        QT_CHECK(strcmp(glfwGetWindowTitle(w), "qnx-glfw-renamed") == 0, "title changed");
        glfwSetWindowSize(w, 200, 150);
        for (int i = 0; i < 10; i++) { glfwPollEvents(); sleep_ms(20); }
        glfwGetWindowSize(w, &x, &y);
        QT_CHECK(x == 200 && y == 150, "resized to 200x150");
        glfwWaitEventsTimeout(0.05);
        QT_CHECK(!glfwWindowShouldClose(w), "window still open after event processing");
        glfwDestroyWindow(w);
      }
      glfwTerminate();
      QT_CHECK(waitpid(lp, NULL, WNOHANG) == 0, "labwc still running after the client left");
    }
  }
  if (!up) {
    FILE *f = fopen(lg, "r"); char line[256];
    if (f) { for (int i = 0; i < 10 && fgets(line, sizeof line, f); i++) printf("  info labwc: %s", line); fclose(f); }
  }
  qx_stop(lp);
  unlink(sock); char lk[310]; snprintf(lk, sizeof lk, "%s.lock", sock); unlink(lk); unlink(lg); rmdir(rt);
}

int main(void) {
  QT_BEGIN("glfw");
  glfwSetErrorCallback(on_error);
  int maj, min, rev; glfwGetVersion(&maj, &min, &rev);
  printf("  info %s\n", glfwGetVersionString());
  QT_CHECK(maj == 3 && min == GLFW_VERSION_MINOR && rev == GLFW_VERSION_REVISION, "library version matches the header");
  QT_CHECK(strstr(glfwGetVersionString(), "Wayland") && strstr(glfwGetVersionString(), "Null"), "built with the Wayland and null platforms");
  null_platform();
  wayland_platform();
  QT_END();
}
