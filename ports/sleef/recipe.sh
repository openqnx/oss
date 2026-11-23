VERSION=3.9.0; GIT=https://github.com/shibatch/sleef.git; COMMIT=906ca7512ee483296780a81a21b9ca715d40dfe1
# SLEEF generates parts of itself with helper programs that must run on the
# build host: a native build of those helpers is made first (do_configure).
# SLEEF_DISABLE_SSL: since 3.9.0 the always-built test_psha programs link
# OpenSSL when found; on the host pass pkg-config led find_package(OpenSSL) to
# the staged QNX libcrypto (undefined __stack_chk_guard, accept@libsocket.so.4).
# OpenSSL is only for SLEEF's own tests, so neither pass looks for it.
do_configure() {
  local nat="$B/native"; mkdir -p "$nat"
  env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u LDFLAGS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH \
      cmake -S "$S" -B "$nat" -G Ninja -DCMAKE_BUILD_TYPE=Release -DSLEEF_BUILD_TESTS=OFF -DSLEEF_DISABLE_SSL=ON \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF && ninja -C "$nat" || return 1
  cmake -S "$S" -B "$B/target" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$arch.cmake" -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$PREFIX" -DNATIVE_BUILD_DIR="$nat" -DSLEEF_BUILD_TESTS=OFF -DSLEEF_DISABLE_SSL=ON -DSLEEF_BUILD_SHARED_LIBS=ON \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF -DSLEEF_BUILD_INLINE_HEADERS=OFF
}
do_build() { ninja -C "$B/target"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B/target"; }
