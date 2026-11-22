VERSION=1.6.0-alpha2; GIT=https://github.com/eclipse-uprotocol/up-spec.git; REF=v1.6.0-alpha.2
# The uProtocol data model (UUri, UUID, UAttributes, UMessage, UStatus) and
# core service interfaces (uSubscription, uDiscovery, uTwin) as Protocol
# Buffers, compiled for C++: the "up-core-api" library up-cpp builds on. The
# up-spec repository has only the .proto files; uProtocol's Conan recipe
# (github.com/eclipse-uprotocol/up-conan-recipes, which qnx-ports'
# up-conan-recipes follows) supplies the CMakeLists.txt -- patches/0001, here
# also a shared library and the CMake package (up-core-api::up-core-api)
# Conan would otherwise generate. 1.6.0-alpha2 is the version that recipe
# pairs with up-cpp 1.0.1.
DEPS="protobuf"
# The .pb.cc files are generated with the protobuf component's build-machine
# protoc (29.3, the runtime's version: protobuf 29 gencode compiles only
# against its own runtime); the staged protoc is a QNX program.
do_configure() {
  local HOST_PROTOC
  HOST_PROTOC=$(find "$BUILD_DIR/host/protobuf" \( -type f -o -type l \) -name protoc | head -1)
  [ -x "$HOST_PROTOC" ] || { echo "no host protoc: build the protobuf component first"; return 1; }
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DBUILD_SHARED_LIBS=ON -DUP_CORE_API_PROTOC="$HOST_PROTOC"
}
