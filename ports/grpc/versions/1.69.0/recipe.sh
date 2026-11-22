VERSION=1.69.0; GIT=https://github.com/grpc/grpc.git; REF=v1.69.0
DEPS="zlib openssl c-ares abseil protobuf re2"
BUILDSYS=custom
# gRPC's build runs protoc and grpc_cpp_plugin to generate its own sources, so
# both must be *host* binaries. Build a native pass first, then point the cross
# configure at the tools it produced.
#
# protobuf, abseil and c-ares come from their own recipes (PROVIDER=package).
# With PROVIDER=module gRPC installed its bundled copies over the tree: a second
# protobuf (29.0 beside 29.3), a second abseil, and c-ares's libcares.pc and
# libcares.a overwritten by the bundled c-ares.  re2 likewise comes from its
# own recipe: as a module, gRPC installed its bundled re2's headers over it.
_HB() { echo "$BUILD_DIR/host/grpc"; }

do_configure() {
  local HB; HB=$(_HB)
  if [ ! -x "$HB/grpc_cpp_plugin" ]; then
    mkdir -p "$HB"
    ( unset CC CXX
      cmake -S "$S" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DgRPC_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF \
        -DgRPC_BUILD_CODEGEN=ON \
        -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
        -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
        -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF )
    cmake --build "$HB" -j"$(nproc)" --target protoc grpc_cpp_plugin
  fi
  local HOST_PROTOC HOST_PLUGIN
  # The protoc that matches the staged runtime -- the protobuf recipe's host
  # build (29.3), not the one bundled with gRPC (29.0).  protobuf 29 gencode
  # #errors unless PROTOBUF_VERSION matches exactly.
  HOST_PROTOC=$(find "$BUILD_DIR/host/protobuf" \( -type f -o -type l \) -name protoc | head -1)
  HOST_PLUGIN=$(find "$HB" \( -type f -o -type l \) -name grpc_cpp_plugin | head -1)
  [ -x "$HOST_PROTOC" ] && [ -x "$HOST_PLUGIN" ] || { echo "host codegen tools missing"; return 1; }

  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DBUILD_SHARED_LIBS=ON -DgRPC_BUILD_TESTS=OFF \
    -DgRPC_INSTALL=ON -DgRPC_BUILD_CODEGEN=OFF \
    -DgRPC_ZLIB_PROVIDER=package -DZLIB_ROOT="$STAGE$PREFIX" \
    -DgRPC_SSL_PROVIDER=package -DOPENSSL_ROOT_DIR="$STAGE$PREFIX" \
    -DgRPC_PROTOBUF_PROVIDER=package -DgRPC_ABSL_PROVIDER=package \
    -DgRPC_CARES_PROVIDER=package -DgRPC_RE2_PROVIDER=package \
    -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl" \
    -DProtobuf_DIR="$STAGE$PREFIX/lib/cmake/protobuf" \
    -Dc-ares_DIR="$STAGE$PREFIX/lib/cmake/c-ares" \
    -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
    -D_gRPC_PROTOBUF_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -DProtobuf_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -DgRPC_CPP_PLUGIN_EXECUTABLE="$HOST_PLUGIN" \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
}
do_build()   { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
