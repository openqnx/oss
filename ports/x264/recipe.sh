VERSION=stable; GIT=https://code.videolan.org/videolan/x264.git; REF=stable
BUILDSYS=custom
# x264 has a hand-written configure that takes --host/--cross-prefix rather than
# the autoconf spelling, and it probes by compiling, so it needs the sysroot.
do_configure() {
  # x264's aarch64 assembly is plain GNU as, so it builds here. Only the x86
  # paths need nasm, which this host lacks. --disable-asm is also a poorly
  # maintained path in x264 -- it drops struct members the C code still uses
  # ("x264_pixel_function_t has no member named ads"), so never take it on
  # aarch64.
  local asmopt=()
  [ "$QNX_ARCH" = x86_64 ] && { command -v nasm >/dev/null || asmopt=( --disable-asm ); }
  cd "$B"
  "$S/configure" --prefix="$PREFIX" \
    --host="$QNX_TRIPLE" \
    --cross-prefix="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
    --sysroot="$QNX_TARGET" \
    --enable-shared --disable-cli --disable-opencl \
    --extra-cflags="-D_QNX_SOURCE" \
    --extra-ldflags="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
    "${asmopt[@]}"
}
do_build()   { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
