# Qt 6 qtremoteobjects, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtremoteobjects-everywhere-src-6.8.1.tar.xz
SHA256=7ef2870f05614b71d1cfdd7ac12acef5294abc43da244a6e9e411f21208f59f8
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
# repc turns .rep interface files into source/replica C++ at build time
# (qt6_add_repc_*): a build-machine tool, so the module is built for the host
# Qt first.
do_configure() { qt6_host_module libexec/repc && qt6_module_configure; }
