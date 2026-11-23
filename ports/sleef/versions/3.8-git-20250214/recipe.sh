VERSION=3.8-git-20250214; GIT=https://github.com/shibatch/sleef.git; COMMIT=b8abbd56df03eb7cc65b8025495b955ce5336973
# SLEEF generates parts of itself with helper programs that must run on the
# build host: a native build of those helpers is made first (do_configure).
do_configure() {
  local nat="$B/native"; mkdir -p "$nat"
  env -u CC -u CXX cmake -S "$S" -B "$nat" -G Ninja -DCMAKE_BUILD_TYPE=Release -DSLEEF_BUILD_TESTS=OFF \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF && ninja -C "$nat" || return 1
  cmake -S "$S" -B "$B/target" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$arch.cmake" -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$PREFIX" -DNATIVE_BUILD_DIR="$nat" -DSLEEF_BUILD_TESTS=OFF -DSLEEF_BUILD_SHARED_LIBS=ON \
      -DSLEEF_BUILD_DFT=OFF -DSLEEF_BUILD_QUAD=OFF -DSLEEF_BUILD_GNUABI_LIBS=OFF -DSLEEF_BUILD_INLINE_HEADERS=OFF
}
do_build() { ninja -C "$B/target"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B/target"; }
