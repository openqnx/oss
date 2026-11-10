# Qt 6 qtnetworkauth, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtnetworkauth-everywhere-src-6.8.1.tar.xz
SHA256=118664ba929c5fbbdf822438bd69bd43674cfee82e504e143da600fe47c74024
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
