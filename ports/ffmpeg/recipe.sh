VERSION=7.1.1; GIT=https://github.com/FFmpeg/FFmpeg.git; REF=n7.1.1
DEPS="zlib bzip2 xz opus libvorbis libogg dav1d openh264 x264 x265 libvpx libtheora lame"
BUILDSYS=custom
# FFmpeg has its own configure with a qnx target-os case; it wants the cross
# prefix rather than CC/CXX, and it must run from the build directory.
do_configure() {
  local arch extra=()
  case "$QNX_ARCH" in
    aarch64le) arch=aarch64 ;;
    x86_64)    arch=x86_64
               # nasm now comes from hosttools/, so the x86 assembly builds.
               command -v nasm >/dev/null || extra+=( --disable-x86asm ) ;;
  esac
  cd "$B"
  "$S/configure" \
    --prefix="$PREFIX" \
    --enable-cross-compile --target-os=qnx --arch="$arch" \
    --cross-prefix="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
    --sysroot="$QNX_TARGET" \
    `# with --cross-prefix set, configure looks for <prefix>pkg-config and,
     # not finding one, silently falls back to "false" -- every pkg-config
     # probe then fails with "not found using pkg-config".` \
    --pkg-config=pkg-config \
    --extra-cflags="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
    --extra-ldflags="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -L$QNX_TARGET/$QNX_ARCH/usr/lib -L$QNX_TARGET/$QNX_ARCH/lib -shared-libgcc" \
    --extra-libs="-lsocket -lm" \
    --enable-shared --disable-static \
    --enable-gpl --enable-version3 \
    --enable-zlib --enable-bzlib --enable-lzma \
    --enable-libopus --enable-libvorbis \
    --enable-libdav1d --enable-libopenh264 --enable-libx264 --enable-libx265 \
    --enable-libvpx --enable-libtheora --enable-libmp3lame \
    --disable-doc --disable-debug \
    --disable-vaapi --disable-vdpau --disable-xlib --disable-libxcb \
    --disable-sdl2 --disable-alsa --disable-libdrm \
    "${extra[@]}"
}
do_build()   { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
