# Qt 6 qthttpserver, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qthttpserver-everywhere-src-6.8.1.tar.xz
SHA256=5d8cb90fddc359b1c74d1e344656785c418483b185d32d97c12d834ec39b0b27
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
