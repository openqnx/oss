# Qt 6 qttranslations, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qttranslations-everywhere-src-6.8.1.tar.xz
SHA256=635a6093e99152243b807de51077485ceadd4786d4acb135b9340b2303035a4a
DEPS="qt6"
BUILDSYS=custom
# Data only (the .qm catalogs, compiled by the build machine's lrelease):
# Qt loads them, so installing them means having Qt.
RUNTIME_DEPS="qt6"
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
