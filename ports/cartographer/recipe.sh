# Cartographer: Google's real-time 2D and 3D SLAM library (the C++ core:
# local scan matching, submaps, pose-graph optimisation with Ceres, Lua
# configuration, protobuf serialisation) -- a static library, as upstream
# builds it (its internal headers are not installed) -- and its tools. No
# gRPC (cloud) service, no Prometheus metrics, no unit tests.
# 2.0.0 is the last release (2021): it predates protobuf 22+ (patches/0001),
# abseil 2024 (0002) and Ceres 2.2 (0003), which this tree has. Lua 5.3 for
# the configuration files, as Debian builds it. Boost: iostreams (gzip
# streams of the .pbstream files); cairo: the map images.
VERSION=2.0.0; GIT=https://github.com/cartographer-project/cartographer.git; REF=2.0.0
DEPS="abseil boost ceres-solver eigen glog gflags lua5.3 protobuf cairo"
_host_protoc() { find "$BUILD_DIR/host/protobuf" -maxdepth 1 -name 'protoc*' -type f -perm -u+x | head -1; }
CMAKE_ARGS_EXTRA=( -DBUILD_GRPC=OFF -DBUILD_PROMETHEUS=OFF
                   -DCARTOGRAPHER_BUILD_TESTS=OFF
                   # The build runs protoc: the build machine's, of the staged
                   # runtime's version (protobuf's host build).
                   -DPROTOBUF_PROTOC_EXECUTABLE="$(_host_protoc)"
                   -Dprotobuf_DIR="$STAGE$PREFIX/lib/cmake/protobuf"
                   -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl"
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
