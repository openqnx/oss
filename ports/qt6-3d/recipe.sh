# Qt 6 qt3d, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qt3d-everywhere-src-6.8.1.tar.xz
SHA256=08b006871b7bad92597c6b588ca921f630131e579e25079e49c0b34c0d7c4236
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
# Model loading (the assimp scene parser plugin) with Qt's bundled Assimp
# 5.x, built into that plugin: Qt 3D asks for Assimp 5 (WrapQt3DAssimp 5),
# and ports/assimp is 6.0.
do_configure() { qt6_module_configure -DINPUT_assimp=qt; }
