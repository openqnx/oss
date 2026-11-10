# Qt 6 qtquickeffectmaker, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtquickeffectmaker-everywhere-src-6.8.1.tar.xz
SHA256=2d4fdf13356abd90b272cc1836bccb6d6ec8849b72173ea3daef0a5c42fb51f6
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
