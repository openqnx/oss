# Qt 6 qtwebchannel, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtwebchannel-everywhere-src-6.8.1.tar.xz
SHA256=6b139f7f9128309e924da145023fa8ea87922bd79693392b3b2e4ba568a5763a
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
