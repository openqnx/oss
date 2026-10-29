# OpenTTD (open source Transport Tycoon Deluxe), at the upstream commit
# qnx-ports' port is based on (patches/: QNX support); part of qnx-ports'
# RetroPie set. OpenTTD generates sources with tools it builds first, so a
# native build of those tools is made (HOST_BINARY_DIR).
VERSION=15.0-git-20250501; GIT=https://github.com/OpenTTD/OpenTTD.git; COMMIT=41b113840a983e2e9b107a452f800eb94c662b23
DEPS="sdl2 zlib xz libpng freetype fontconfig curl lzo"
BUILDSYS=custom
do_configure() {
  local nat="$B/native"; mkdir -p "$nat"
  env -u CC -u CXX -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR cmake -S "$S" -B "$nat" -G Ninja -DOPTION_TOOLS_ONLY=ON && \
    cmake --build "$nat" --target tools || return 1
  # C++20 for every target: the toolchain's default flags say -std=gnu++17,
  # which openttd_test (no standard of its own) kept, and std::source_location
  # is C++20 ("source_location in namespace std does not name a type").
  cmake -S "$S" -B "$B/target" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DOPTION_TOOLS_ONLY=OFF -DHOST_BINARY_DIR="$nat" \
    -DCMAKE_DISABLE_FIND_PACKAGE_ICU=ON -DCPU="$QNX_CPU"
}
do_build() { cmake --build "$B/target" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B/target"; }
