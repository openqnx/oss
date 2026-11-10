VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/single/qt-everywhere-src-6.8.1.tar.xz
SHA256=45e3a9f6d33c92ffe65a1fde1a8eba5b228112df675f7f9026eaa332b2e2edff
DEPS="zlib libpng libjpeg-turbo freetype harfbuzz pcre2 openssl icu double-conversion ffmpeg fontconfig gstreamer gst-plugins-base gst-plugins-bad libtiff libwebp zstd wayland libxkbcommon"
BUILDSYS=custom
# Qt 6.8.1, the release qnx-ports builds. Qt supports QNX as a target itself
# (QPA plugin "qnx" on Screen + EGL/GLES2 from the SDP); the source patches
# move Qt Multimedia's audio onto SDP 8's ALSA API and let QtGui find Wayland
# and xkbcommon on QNX (patches/).
# Modules: the commonly used set rather than all 40 (QtWebEngine, for one, does
# not support QNX). configure adds what these need: Qt Quick 3D, Quick
# Timeline (qtmultimedia's spatial audio) and Qt Language Server (qttools'
# qmlls) are in this package too. The other add-on modules are ports of their
# own, built against this one: ports/qt6-<module> (scripts/lib/qt6-module.sh).
#
# A Qt cross build needs a host Qt of the same version for its tools (moc, rcc,
# uic, qsb, ...); it is built from the same source into hosttools/qt6-$VERSION,
# once for both architectures.
QT_SUBMODULES="qtbase,qtshadertools,qtdeclarative,qtsvg,qtimageformats,qt5compat,qtserialport,qtserialbus,qtwebsockets,qtcharts,qtvirtualkeyboard,qttools,qtmultimedia"
QT_HOST="$WS/hosttools/qt6-$VERSION"
# Qt's QNX mkspecs: qnx-aarch64le-qcc and qnx-x86-64-qcc (dashes, not the
# x86_64 of the architecture name: "Unknown platform qnx-x86_64-qcc").
_qt_mkspec() { case "$QNX_ARCH" in aarch64le) echo qnx-aarch64le-qcc ;; x86_64) echo qnx-x86-64-qcc ;; esac; }
# EGLFS and KMS are off: they are Linux's DRM/KMS display integration; on QNX Qt
# draws through its qnx platform plugin (Screen).
# backtrace is off: QNX has no glibc backtrace() ("'::backtrace' has not been
# declared" in qlogging.cpp).
# wayland and libxkbcommon: QtGui's wayland feature (the QWaylandApplication
# native interface), without which Qt Wayland's client (ports/qt6-wayland,
# Qt programs on labwc) does not build ("Qt Gui has been built without
# 'wayland' feature"), and xkbcommon, which the wayland client maps keyboard
# input with. Wayland itself is only headers here: QtGui does not link it.
# What else configure would now find in staging stays off, as it was before
# (Qt's xcb plugin, the GTK 3 theme, the AT-SPI bridge, the Linux input
# drivers, CUPS printing; Qt's own copies of libb2 and md4c -- staging's md4c
# is newer than Qt 6.8 expects: "static assertion failed" in
# qtextmarkdownimporter.cpp): -no-xcb -no-feature-gtk3
# -no-feature-accessibility-atspi-bridge -no-libinput -no-mtdev -no-tslib
# -no-evdev -no-feature-system-libb2 -no-feature-system-textmarkdownreader
# -no-feature-cups.
# QT_HOST_PATH_CMAKE_DIR spelled like -qt-host-path: a rebuild over the
# staged Qt otherwise takes the host CMake directory that staged Qt recorded,
# from a workspace reached through another path, and CMake's re-rooting then
# misses the host tools ("Failed to find the host tool Qt6::qsb";
# docs/porting-hazards.md).
# qdoc (and the libclang it needs) is off for the target as it is for the
# host build: the target configure otherwise looks for the host's qdoc
# ("Failed to find the host tool Qt6::qdoc").
do_configure() {
  if [ ! -x "$QT_HOST/bin/qmake" ]; then
    local hb="$BUILD_DIR/host/qt6-$VERSION"; mkdir -p "$hb"
    ( cd "$hb" && env -u CC -u CXX -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR "$S/configure" \
        -prefix "$QT_HOST" -submodules "$QT_SUBMODULES" -release -nomake examples -nomake tests \
        -no-opengl -no-feature-vulkan -- -G Ninja && cmake --build . --parallel && cmake --install . ) || return 1
  fi
  # Qt derives QNX itself, from qt_set01(QNX CMAKE_SYSTEM_NAME STREQUAL "QNX"),
  # whose unquoted ${ARGN} reads the literal QNX as a variable: with the
  # shared toolchain's set(QNX TRUE) in force it compares against "TRUE" and
  # turns QNX off, so neither the qnx platform plugin nor qtmultimedia's QNX
  # audio backend is built ("qqnxmediadevices_p.h: No such file or
  # directory"). Wrap the toolchain and drop QNX for Qt to set.
  printf 'include("%s")\nunset(QNX)\n' "$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" > "$B/qt-toolchain.cmake"
  ( cd "$B" && "$S/configure" -prefix "$PREFIX" -qt-host-path "$QT_HOST" \
      -submodules "$QT_SUBMODULES" -release -nomake examples -nomake tests \
      -opengl es2 -qpa qnx -system-zlib -system-libpng -system-libjpeg -system-freetype -system-harfbuzz \
      -system-pcre -openssl-linked -icu -no-feature-qdoc -no-feature-clang -no-feature-backtrace -no-eglfs -no-kms -feature-alsa -no-sbom \
      -feature-wayland -xkbcommon -no-xcb -no-feature-gtk3 -no-feature-accessibility-atspi-bridge -no-libinput -no-mtdev -no-tslib -no-evdev \
      -no-feature-system-libb2 -no-feature-system-textmarkdownreader -no-feature-cups \
      -- -G Ninja -DCMAKE_TOOLCHAIN_FILE="$B/qt-toolchain.cmake" -DQT_QMAKE_TARGET_MKSPEC="$(_qt_mkspec)" \
      -DQT_HOST_PATH_CMAKE_DIR="$QT_HOST/lib/cmake" )
}
do_build() { cmake --build "$B" --parallel; }
# STAGE is the component's pkgroot while installing, so install there.
# SBOM generation is off (-no-sbom): at install time it checksums files at the
# configured prefix, ignoring --prefix ("Cannot find 'lib/libQt6Core.so.6.8.1'
# to compute its checksum"), and with DESTDIR it then could not find the
# other modules' SBOM documents ("Could not find external SBOM document").
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
