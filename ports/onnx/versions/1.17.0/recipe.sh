VERSION=1.17.0; GIT=https://github.com/onnx/onnx.git; REF=v1.17.0
DEPS="protobuf"
# ONNX generates its schema sources with protoc while it builds, so it needs a
# protoc that runs on the build host -- and protobuf 29 gencode compiles only
# against the exact same runtime version, so it must be the host protoc the
# protobuf recipe builds (29.3), not a distribution one. The code generator
# scripts are Python and need the host interpreter, which the QNX toolchain
# file would otherwise look for in the target sysroot.
_host_protoc() {
  local HB="$BUILD_DIR/host/protobuf" p
  p=$(find "$HB" -maxdepth 1 -name 'protoc*' -type f -perm -u+x 2>/dev/null | head -1)
  if [ -z "$p" ]; then
    local PS="$SRC_DIR/protobuf"
    [ -d "$PS" ] || git clone -q --depth 1 -b v29.3 --recurse-submodules https://github.com/protocolbuffers/protobuf.git "$PS"
    cmake -S "$PS" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF \
      -Dprotobuf_ABSL_PROVIDER=module -DBUILD_SHARED_LIBS=OFF >&2 && cmake --build "$HB" --target protoc >&2
    p=$(find "$HB" -maxdepth 1 -name 'protoc*' -type f -perm -u+x | head -1)
  fi
  echo "$p"
}
do_configure() {
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DONNX_BUILD_TESTS=OFF -DONNX_USE_PROTOBUF_SHARED_LIBS=ON -DBUILD_ONNX_PYTHON=OFF -DONNX_USE_LITE_PROTO=OFF \
    -DONNX_WERROR=OFF -DBUILD_SHARED_LIBS=ON -DProtobuf_PROTOC_EXECUTABLE="$(_host_protoc)" \
    -DPython3_EXECUTABLE=/usr/bin/python3 -DPYTHON_EXECUTABLE=/usr/bin/python3
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
