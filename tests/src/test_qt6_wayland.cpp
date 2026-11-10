// Qt Wayland (client): the wayland platform plugins (generic, EGL) and xdg-shell
// integration load, with everything they link (libQt6WaylandClient,
// libwayland-client, -cursor, xkbcommon through QtGui), and say what they
// are. Then a real session: labwc (the compositor of the QNX desktop) on
// wlroots' headless backend, which needs no display, and a QGuiApplication
// on the wayland platform opening an xdg-shell window there that the
// compositor configures and exposes, with its keymap from the compositor
// (xkbcommon), painted through wl_shm buffers.
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QLibraryInfo>
#include <QPainter>
#include <QPluginLoader>
#include <QRasterWindow>
#include <QStandardPaths>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

class Red : public QRasterWindow {
 public:
  int painted = 0;
 protected:
  void paintEvent(QPaintEvent*) override { QPainter(this).fillRect(QRect(0, 0, width(), height()), QColor(200, 0, 0)); ++painted; }
};

static QStringList keys(QPluginLoader& l) {
  QStringList k;
  for (const QJsonValue& v : l.metaData().value("MetaData").toObject().value("Keys").toArray()) k << v.toString();
  return k;
}

int main(int argc, char** argv) {
  QT_BEGIN("qt6-wayland");
  const QString plugins = QLibraryInfo::path(QLibraryInfo::PluginsPath);

  QPluginLoader qpa(plugins + "/platforms/libqwayland-generic.so");
  bool loaded = qpa.load();
  if (!loaded) printf("  info %s\n", qPrintable(qpa.errorString()));
  QT_CHECK(loaded, "platforms/libqwayland-generic.so loads (Qt6WaylandClient, libwayland-client, -cursor)");
  QT_CHECK(keys(qpa).contains("wayland") &&
           qpa.metaData().value("IID").toString().contains("QPlatformIntegrationFactoryInterface"),
           "QPA plugin, key wayland");
  QPluginLoader egl(plugins + "/platforms/libqwayland-egl.so");
  bool el = egl.load();
  if (!el) printf("  info %s\n", qPrintable(egl.errorString()));
  QT_CHECK(el && keys(egl).contains("wayland-egl"), "platforms/libqwayland-egl.so loads (EGL on Wayland), key wayland-egl");
  QPluginLoader xdg(plugins + "/wayland-shell-integration/libxdg-shell.so");
  bool xl = xdg.load();
  if (!xl) printf("  info %s\n", qPrintable(xdg.errorString()));
  QT_CHECK(xl && keys(xdg).contains("xdg-shell"), "xdg-shell integration plugin loads");

  // labwc, headless, in a runtime directory of its own.
  char rt[] = "/tmp/qt6-wayland-rt-XXXXXX";
  QT_CHECK(mkdtemp(rt) != nullptr, "XDG_RUNTIME_DIR");
  chmod(rt, 0700);
  setenv("XDG_RUNTIME_DIR", rt, 1);
  setenv("WLR_BACKENDS", "headless", 1);
  setenv("WLR_RENDERER", "pixman", 1);
  unsetenv("WAYLAND_DISPLAY");
  pid_t lp = fork();
  if (lp == 0) { execlp("labwc", "labwc", (char*)nullptr); _exit(127); }
  const QString sock = QString::fromLatin1(rt) + "/wayland-0";
  bool up = false;
  for (int i = 0; i < 150 && !(up = QFileInfo::exists(sock)); ++i) usleep(100000);
  QT_CHECK(up, "labwc (headless) is up: wayland-0");
  if (up) {
    setenv("WAYLAND_DISPLAY", "wayland-0", 1);
    setenv("QT_QPA_PLATFORM", "wayland", 1);
    QGuiApplication app(argc, argv);
    QT_CHECK(QGuiApplication::platformName() == "wayland", "QGuiApplication on the wayland platform");
    Red w;
    w.setTitle("qt6-wayland test");
    w.resize(320, 200);
    w.show();
    for (int i = 0; i < 100 && !(w.isExposed() && w.painted); ++i) pump(50);
    QT_CHECK(w.isExposed(), "xdg-shell window configured and exposed by labwc");
    QT_CHECK(w.painted > 0, "window painted (wl_shm buffer)");
    QT_CHECK(w.width() == 320 && w.height() == 200, "window size 320 x 200");
    w.close();
    pump(100);
  }
  int st;
  if (lp > 0) {
    kill(lp, SIGTERM);
    bool ok = false;
    for (int i = 0; i < 50 && !ok; ++i) { if (waitpid(lp, &st, WNOHANG) == lp) ok = true; else usleep(100000); }
    if (!ok) { kill(lp, SIGKILL); waitpid(lp, &st, 0); }
  }
  unlink(qPrintable(sock)); unlink(qPrintable(sock + ".lock")); rmdir(rt);
  QT_END();
}
