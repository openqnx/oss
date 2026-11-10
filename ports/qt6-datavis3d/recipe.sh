# Qt 6 qtdatavis3d, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtdatavis3d-everywhere-src-6.8.1.tar.xz
SHA256=dae68fe06db8aa3adedb3067c733352bcb29e42c8afcdc02cfda6755da504b0e
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
