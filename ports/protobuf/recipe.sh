VERSION=29.3; GIT=https://github.com/protocolbuffers/protobuf.git; REF=v29.3
DEPS="zlib abseil"
# protoc must run on the *host* during the build, so a native protoc is built
# first and handed to the cross build via protobuf_PROTOC_EXE.
BUILDSYS=custom
# ABSL_PROVIDER=package: link the staged abseil rather than installing the
# submodule's copy (20240116) next to it -- see ports/abseil.  The host protoc
# below keeps the module: it is a static build machine tool and installs
# nothing.  gRPC's cross build uses that same host protoc, because protobuf 29
# gencode compiles only against the exact runtime version
# ("#if PROTOBUF_VERSION != ..." in compiler/cpp/file.cc).
_common=( -DBUILD_SHARED_LIBS=ON -Dprotobuf_BUILD_TESTS=OFF
          -Dprotobuf_BUILD_EXAMPLES=OFF -Dprotobuf_ABSL_PROVIDER=package
          -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl"
          -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
do_configure() {
  local HB="$BUILD_DIR/host/protobuf"
  if [ ! -x "$HB/protoc" ]; then
    mkdir -p "$HB"
    cmake -S "$S" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_EXAMPLES=OFF \
      -Dprotobuf_ABSL_PROVIDER=module -DBUILD_SHARED_LIBS=OFF
    cmake --build "$HB" -j"$(nproc)" --target protoc
  fi
  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -Dprotobuf_PROTOC_EXE="$(find "$HB" -name protoc -type f | head -1)" \
    "${_common[@]}"
}
do_build()   { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
