# opentelemetry-cpp: the OpenTelemetry C++ client -- the tracing, metrics and
# logs API, the SDK behind it, and exporters.
#
# Built (shared libraries): API, SDK, the ostream and in-memory exporters, and
# the OTLP exporters over HTTP (curl, protobuf, nlohmann_json), over gRPC
# (the ported grpc/protobuf/abseil) and to files; gzip compression of OTLP/HTTP
# requests (zlib).
# Left out: Prometheus, Zipkin, Elasticsearch, the OpenTracing shim and the
# YAML configuration (each needs a library that is not ported), examples,
# tests and benchmarks.
#
# The OTLP protocol classes are generated at build time from the
# opentelemetry-proto submodule, so protoc and grpc_cpp_plugin must be build
# machine programs: the ones the protobuf and grpc recipes built for
# themselves (build/host/protobuf, build/host/grpc).  protobuf 29 gencode only
# compiles against the exact runtime version, which is why it has to be that
# protoc (29.3) and not any other one on the PATH.
VERSION=1.30.0; GIT=https://github.com/open-telemetry/opentelemetry-cpp.git; REF=v1.30.0
DEPS="protobuf grpc abseil curl nlohmann_json zlib openssl c-ares re2"
BUILDSYS=cmake
_HOST_PROTOC=$(find "$BUILD_DIR/host/protobuf" -maxdepth 1 \( -type f -o -type l \) -name protoc 2>/dev/null | head -1)
_HOST_GRPC_PLUGIN=$(find "$BUILD_DIR/host/grpc" -maxdepth 1 \( -type f -o -type l \) -name grpc_cpp_plugin 2>/dev/null | head -1)
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DCMAKE_CXX_STANDARD=17
                   -DOTELCPP_BUILD_TESTING=OFF -DOTELCPP_WITH_EXAMPLES=OFF
                   -DOTELCPP_WITH_FUNC_TESTS=OFF -DOTELCPP_WITH_BENCHMARK=OFF
                   -DOTELCPP_WITH_OTLP_HTTP=ON -DOTELCPP_WITH_OTLP_GRPC=ON
                   -DOTELCPP_WITH_OTLP_FILE=ON -DOTELCPP_WITH_OTLP_HTTP_COMPRESSION=ON
                   -DOTELCPP_WITH_PROMETHEUS=OFF -DOTELCPP_WITH_ZIPKIN=OFF
                   -DOTELCPP_WITH_ELASTICSEARCH=OFF -DOTELCPP_WITH_OPENTRACING=OFF
                   # Everything comes from the staging tree; a missing package
                   # must fail the configure step, not be downloaded and built.
                   -DFETCHCONTENT_FULLY_DISCONNECTED=ON
                   -DProtobuf_PROTOC_EXECUTABLE="$_HOST_PROTOC"
                   -DgRPC_CPP_PLUGIN_EXECUTABLE="$_HOST_GRPC_PLUGIN" )
