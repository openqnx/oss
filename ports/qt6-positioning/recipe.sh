# Qt 6 qtpositioning, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtpositioning-everywhere-src-6.8.1.tar.xz
SHA256=e310e7232591d4beb1785bfff8ff3e77430bdf5e9a17f56694b732f5267df78d
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
