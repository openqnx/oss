# RetroArch, the libretro frontend, as packaged by RetroPie, at the upstream
# commit qnx-ports' QNX port is based on (patches/: QNX platform support --
# Screen/EGL video, input, QNX build settings). Part of the RetroPie stack.
VERSION=1.15-git-20230408; GIT=https://github.com/RetroPie/RetroArch.git; COMMIT=d625ca376267cef2c304a8a8f033f21003c8df02
DEPS="sdl2 freetype zlib fontconfig fribidi libass openssl"
BUILDSYS=custom
# RetroArch's configure is its own (not autoconf) and builds in the source tree.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc" CXX="q++" CFLAGS="-D_QNX_SOURCE -std=c17 -V$QCC_TARGET -I$STAGE$PREFIX/include" \
  CXXFLAGS="-D_QNX_SOURCE -std=c++17 -V${QCC_TARGET}_cxx -Wno-deprecated-definitions -I$STAGE$PREFIX/include" \
  CPPFLAGS="-D_QNX_SOURCE" LDFLAGS="-V$QCC_TARGET -L$STAGE$PREFIX/lib" \
  PKG_CONF_PATH=pkg-config \
  ./configure --host="$QNX_TRIPLE-" --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --disable-x11 --disable-wayland --disable-builtinzlib --disable-ffmpeg; }
# PKG_CONF_PATH: configure looks for <triple>-pkg-config; the tree's pkg-config
# (PKG_CONFIG_LIBDIR/SYSROOT from env.sh) is the one to use.
# --disable-ffmpeg: this RetroArch (2023, the RetroPie revision qnx-ports
# ports) uses the FFmpeg API before 7.0 (AVCodecContext::channel_layout); the
# tree's FFmpeg is 7. It only provides the ffmpeg core and recording.
# --disable-builtinzlib: the zlib component (a DEPS entry), not the copy in
# deps/libz, whose prototypes clash with the staged zlib.h.
# --disable-x11 --disable-wayland: on QNX RetroArch draws through Screen/SDL2.
# Its Wayland input maps Linux evdev key codes (<linux/input.h>); its X11 drivers
# include the XShm one, which needs SysV shared memory (<sys/shm.h>) that QNX
# does not have.
# EGL_LIBS=-lEGL: configure finds QNX's EGL (HAVE_EGL) through its headers but
# has no pkg-config file to take the library from, so the link missed it.
do_build() { make -C "$B" -j"$(nproc)" EGL_LIBS=-lEGL; }
do_install() { make -C "$B" DESTDIR="$STAGE" EGL_LIBS=-lEGL install; }
