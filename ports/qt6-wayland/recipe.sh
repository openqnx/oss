# Qt 6 qtwayland, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtwayland-everywhere-src-6.8.1.tar.xz
SHA256=2226fbde4e2ddd12f8bf4b239c8f38fd706a54e789e63467dfddc77129eca203
# Keyboard input without xkbcommon: Qt reads xkbcommon only through qtbase's
# xkbcommon feature, which ports/qt6 does not enable; the protocol XML Qt
# needs is in its own source (src/3rdparty/protocol).
DEPS="qt6 wayland"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
# The client side only: the wayland platform plugin (QT_QPA_PLATFORM=wayland,
# what labwc runs Qt programs with), its shell integrations (xdg-shell, ...)
# and the EGL client buffer integration; not the compositor library.
# qtwaylandscanner generates C++ from the protocol XML at build time: the host
# build makes only that tool (no client, no compositor) for the host Qt.
# The protocol XML is turned into C by wayland-scanner from hosttools/.
do_configure() {
  qt6_host_module libexec/qtwaylandscanner -DFEATURE_wayland_client=OFF -DFEATURE_wayland_server=OFF &&
  qt6_module_configure -DFEATURE_wayland_server=OFF -DFEATURE_wayland_client=ON
}
