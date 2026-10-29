VERSION=76.1
# The release tarball, not the git tag: only the tarball carries the prebuilt
# data archive (source/data/in/icudt76l.dat). From git the build quietly
# produced the 7 KB stub libicudata, and every ICU user on the target had no
# locale data (Node.js: "Icu error" from Intl).
TARBALL=https://github.com/unicode-org/icu/releases/download/release-76-1/icu4c-76_1-src.tgz
SHA256=dfacb46bfe4747410472ce3e1144bf28a102feeaa4e3875bac9b4c6cf30f4f3e
SUBDIR=source
BUILDSYS=custom
# ICU's cross build needs a *native* ICU build first: its data and message
# tooling (pkgdata, genrb, ...) runs on the build machine, and configure wants
# that tree handed to it via --with-cross-build.
do_configure() {
  local HB="$BUILD_DIR/host/icu"
  if [ ! -f "$HB/config/icucross.mk" ]; then
    rm -rf "$HB"; mkdir -p "$HB"
    ( cd "$HB" && unset CC CXX CPP CFLAGS CXXFLAGS CPPFLAGS LDFLAGS \
      && "$S/configure" --enable-static --disable-shared --disable-tests \
           --disable-samples --disable-extras \
      && make -j"$(nproc)" ) >> "$LOG_DIR/icu-host.log" 2>&1 \
      || { echo "native ICU build failed -- see $LOG_DIR/icu-host.log"; return 1; }
  fi
  cd "$B"
  CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" \
  CXX="$QNX_HOST/usr/bin/$QNX_TRIPLE-g++" \
  CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
  "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" \
    --with-cross-build="$HB" \
    --enable-shared --disable-static \
    --disable-tests --disable-samples --disable-extras
}
do_build()   { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
