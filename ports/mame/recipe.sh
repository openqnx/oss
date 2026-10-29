VERSION=0.281; GIT=https://github.com/mamedev/mame.git; REF=mame0281
DEPS="sdl2 sdl2-ttf fontconfig freetype zlib"
BUILDSYS=custom
# MAME 0.281 with qnx-ports' QNX support (patches/: a QNX target OS in MAME's
# GENie build scripts and the qnx_x64/qnx_arm64 make targets, OS fixes in
# osd/). Video and audio through SDL2. A full build of every driver is very
# large (hours, tens of GB); set MAME_SUBTARGET (e.g. "tiny") to build a
# subset.
MAME_SUBTARGET=${MAME_SUBTARGET:-}
# -Wno-error=cpp: SDL_config.h (sdl2-compat's, when that is the SDL2 in the
# tree) carries a #warning, which MAME's -Werror made fatal in osd/sdl.
do_build() {
  local t=$([ "$QNX_ARCH" = aarch64le ] && echo qnx_arm64 || echo qnx_x64)
  cd "$S" && PKG_CONFIG_PATH= make "$t" -j"$(nproc)" TARGETOS=qnx TESTS=0 SYMBOLS=0 DEBUG=0 \
    ARCHOPTS="-D_QNX_SOURCE -DSDL_DISABLE_IMMINTRIN_H -Wno-error=cpp -DCATCH_CONFIG_CPP11_NO_SHUFFLE -Wno-error=format-overflow -Wno-error=alloc-size-larger-than= -Wno-error=parentheses -Wno-error=pointer-arith -Wno-error=unused-value" \
    LDOPTS="-Wl,-rpath-link=$STAGE$PREFIX/lib -L$STAGE$PREFIX/lib -lsocket" SDL_INSTALL_ROOT="$STAGE$PREFIX" \
    USE_QTDEBUG=0 USE_TAPTUN=1 NO_USE_XINPUT=1 NO_X11=1 NO_USE_MIDI=1 USE_SYSTEM_LIB_ZLIB=1 \
    BUILDDIR="$(realpath -m --relative-to="$S" "$B/build")" ${MAME_SUBTARGET:+SUBTARGET=$MAME_SUBTARGET}
}
# BUILDDIR is given relative to the source tree: GENie writes the project
# files under an absolute BUILDDIR with its leading "/" dropped
# ("home/qnx/.../gmake-nto-x86_64-o"), and make then cannot find them.
do_install() {
  local exe=mame; [ -n "$MAME_SUBTARGET" ] && exe=$MAME_SUBTARGET
  install -d "$STAGE$PREFIX/bin" && install -m755 "$S/$exe" "$STAGE$PREFIX/bin/" && rm -f "$S/$exe"
}
