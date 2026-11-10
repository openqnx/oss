# Helpers for the Qt 6 add-on modules (ports/qt6-<module>): one Qt submodule,
# from its own release tarball of the version ports/qt6 builds, cross-built
# against the staged qt6 the way Qt builds a module on top of an installed Qt
# (CMake, find_package(Qt6), QT_HOST_PATH for the host tools). Recipe:
#   VERSION=6.8.1  TARBALL=.../submodules/<module>-everywhere-src-6.8.1.tar.xz  SHA256=...
#   DEPS="qt6 ..."  BUILDSYS=custom  source "$WS/scripts/lib/qt6-module.sh"
#   do_configure() { qt6_module_configure [-D...]; }   (build/install: below)
# A module with host tools of its own (qscxmlc, repc, qtwaylandscanner) first
# builds them for the build machine into hosttools/qt6-<version>, next to the
# host Qt ports/qt6 builds there: qt6_host_module <tool-file> [-D...].
QT6_VERSION=$(sed -n 's/^VERSION=\([^;]*\).*/\1/p' "$WS/ports/qt6/recipe.sh")
QT_HOST="$WS/hosttools/qt6-$QT6_VERSION"

# The same source, built for the build machine against the host Qt and
# installed into it, unless <tool-file> (relative to the host Qt) is there.
qt6_host_module() {
  local tool=$1; shift
  [ -e "$QT_HOST/$tool" ] && return 0
  [ -x "$QT_HOST/bin/qmake" ] || { echo "no host Qt in $QT_HOST: build qt6 first"; return 1; }
  local hb="$BUILD_DIR/host/$NAME-$VERSION"; rm -rf "$hb"; mkdir -p "$hb"
  # The cross environment off: host compiler, host pkg-config.
  env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u LDFLAGS -u PKG_CONFIG -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR \
    cmake -S "$S" -B "$hb" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QT_HOST" \
      -DCMAKE_INSTALL_PREFIX="$QT_HOST" -DQT_BUILD_EXAMPLES=OFF -DQT_BUILD_TESTS=OFF -DQT_GENERATE_SBOM=OFF "$@" \
  && env -u CC -u CXX cmake --build "$hb" --parallel && cmake --install "$hb" && [ -e "$QT_HOST/$tool" ]
}

# Qt derives QNX itself (qt_set01(QNX CMAKE_SYSTEM_NAME STREQUAL "QNX")); the
# shared toolchain's set(QNX TRUE) turns that into a comparison with "TRUE"
# and QNX off (ports/qt6/recipe.sh says more). Same wrapper as ports/qt6.
# QT_HOST_PATH_CMAKE_DIR given too: the staged Qt remembers the one it was
# configured with, and a workspace reached through another path (a symlink)
# then fails CMake's re-rooting check -- the host tools packages of a module
# (Qt6ScxmlTools) are searched under the staging tree and not found.
# Examples and tests off; SBOM off as in ports/qt6 (it checksums files at the
# configured prefix, not in DESTDIR).
qt6_module_configure() {
  mkdir -p "$B"
  printf 'include("%s")\nunset(QNX)\n' "$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" > "$B/qt-toolchain.cmake"
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$B/qt-toolchain.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DQT_HOST_PATH="$QT_HOST" -DQT_HOST_PATH_CMAKE_DIR="$QT_HOST/lib/cmake" -DQt6_DIR="$STAGE_SYSROOT$PREFIX/lib/cmake/Qt6" \
    -DQT_BUILD_EXAMPLES=OFF -DQT_BUILD_TESTS=OFF -DQT_GENERATE_SBOM=OFF "$@"
}
do_build() { cmake --build "$B" --parallel; }
# STAGE is the component's pkgroot while installing.
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
