# Apache Avro C++: Avro schemas, binary and JSON encoders/decoders, generic
# datums, object container files (null, deflate, snappy and zstd codecs) and
# avrogencpp, the schema-to-C++ code generator.
# The release tag of the Avro tree, C++ in lang/c++. Not the avro-cpp release
# tarball: 1.12.2's lacks cmake/avro-cpp-config.cmake.in, which
# CMakeLists.txt configures, so it does not build as released.
VERSION=1.12.2; GIT=https://github.com/apache/avro.git; REF=release-1.12.2
SUBDIR=lang/c++
# fmt is used header-only at build time (no run-time dependency); without a
# staged fmt, CMakeLists.txt would FetchContent fmt 10.2.1 from GitHub.
DEPS="fmt zlib snappy zstd"
# The tests need Boost; the library itself does not (AVRO_USE_BOOST=OFF).
CMAKE_ARGS_EXTRA=( -DAVRO_BUILD_TESTS=OFF -DAVRO_BUILD_STATIC=OFF -DAVRO_BUILD_SHARED=ON
                   -DAVRO_BUILD_EXECUTABLES=ON -DAVRO_USE_BOOST=OFF )
