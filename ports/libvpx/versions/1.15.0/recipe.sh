VERSION=1.15.0; GIT=https://chromium.googlesource.com/webm/libvpx; REF=v1.15.0
BUILDSYS=custom
# libvpx has a hand-written configure driven by a --target triple of its own.
do_configure() {
  local tgt extra=()
  case "$QNX_ARCH" in
    aarch64le) tgt=arm64-linux-gcc      # generic ELF/aarch64 profile; CROSS gives the real toolchain
               # libvpx's aarch64 runtime CPU detection reads Linux hwcaps
               # (getauxval); there is no QNX method, and the file says so:
               # "Rerun configure with --disable-runtime-cpu-detect". Without
               # detection every extension that is compiled in is called
               # unconditionally, so keep to what all ARMv8.0 parts have: the
               # dotprod/i8mm/SVE paths died with SIGILL on a Cortex-A57.
               extra+=( --disable-runtime-cpu-detect --disable-neon_dotprod --disable-neon_i8mm
                        --disable-sve --disable-sve2 ) ;;
    x86_64)    tgt=x86_64-linux-gcc ;;  # x86 detection is cpuid, which works anywhere
  esac
  cd "$B"
  CROSS="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
  CFLAGS="-D_QNX_SOURCE" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
  "$S/configure" --prefix="$PREFIX" --target="$tgt" \
    --enable-pic --enable-shared --disable-static \
    --disable-examples --disable-tools --disable-docs --disable-unit-tests \
    --enable-vp8 --enable-vp9 --disable-webm-io \
    "${extra[@]}"
}
do_build()   { make -C "$B" -j"$(nproc)"; }
# libvpx installs into DIST_DIR, not DESTDIR: passing DESTDIR alone silently
# produces no files in the staging tree.
do_install() { make -C "$B" DIST_DIR="$STAGE$PREFIX" install; }
