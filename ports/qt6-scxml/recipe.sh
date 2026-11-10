# Qt 6 qtscxml, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtscxml-everywhere-src-6.8.1.tar.xz
SHA256=2f406f44cfacd6eddb90468012a5d1d99d7a64ec05a21a2a7eda9dfc12614fd6
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
# qscxmlc compiles SCXML documents to C++ at build time (qt6_add_statecharts):
# a build-machine tool, so the module is built for the host Qt first.
do_configure() { qt6_host_module libexec/qscxmlc && qt6_module_configure; }
