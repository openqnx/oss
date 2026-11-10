# Qt 6 qtsensors, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtsensors-everywhere-src-6.8.1.tar.xz
SHA256=41f49b614850d40c647b80e70ef6be759e8fc90ac6cce3ab6f82a357201d9750
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
