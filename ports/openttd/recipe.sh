# OpenTTD (open source Transport Tycoon Deluxe), at the upstream commit
# qnx-ports' port is based on (patches/: QNX support); part of qnx-ports'
# RetroPie set. OpenTTD generates sources with tools it builds first, so a
# native build of those tools is made (HOST_BINARY_DIR).
# The binary goes to bin/ (OpenTTD defaults to games/, which is not on PATH,
# so the desktop entry's Exec=openttd failed).
VERSION=15.3; GIT=https://github.com/OpenTTD/OpenTTD.git; COMMIT=14ec60f248547d4d062a1160f0fc26d742319888
DEPS="sdl2 zlib xz libpng freetype fontconfig curl lzo"
BUILDSYS=custom
do_configure() {
  local nat="$B/native"; mkdir -p "$nat"
  env -u CC -u CXX -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR cmake -S "$S" -B "$nat" -G Ninja -DOPTION_TOOLS_ONLY=ON && \
    cmake --build "$nat" --target tools || return 1
  # -flifetime-dse=1: OpenTTD's pool allocator stores each item's index in
  # operator new, before the constructor runs; GCC's default lifetime DSE
  # drops that store, every Engine got a garbage index and the game crashed
  # at start in ResetNewGRFData(). Upstream adds the flag for GCC, but the
  # qnx-ports patch skips that check on QNX.
  # C++20 for every target: the toolchain's default flags say -std=gnu++17,
  # which openttd_test (no standard of its own) kept, and std::source_location
  # is C++20 ("source_location in namespace std does not name a type").
  cmake -S "$S" -B "$B/target" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -flifetime-dse=1" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_BINDIR=bin -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DOPTION_TOOLS_ONLY=OFF -DHOST_BINARY_DIR="$nat" \
    -DCMAKE_DISABLE_FIND_PACKAGE_ICU=ON -DCPU="$QNX_CPU"
}
do_build() { cmake --build "$B/target" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B/target"; }
