# Qt 6 qtlocation, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtlocation-everywhere-src-6.8.1.tar.xz
SHA256=9070db5eb8bb873a1d4e49d3aa922e37e22771c6636d031d3a9f6addae4b59ef
DEPS="qt6 qt6-positioning"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
