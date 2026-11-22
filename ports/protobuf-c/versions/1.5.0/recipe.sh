VERSION=1.5.0; GIT=https://github.com/protobuf-c/protobuf-c.git; REF=v1.5.0
# The runtime library only. protoc-gen-c is a code generator for the build host.
SUBDIR=build-cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_PROTOC=OFF -DBUILD_TESTS=OFF )
