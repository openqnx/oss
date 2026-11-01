# riegeli: Google's file format for sequences of records (protocol buffer
# messages or plain strings) with compression, checksums and recovery from
# damage -- libriegeli (its readers and writers, records, chunk encodings,
# Brotli/Zstd/Snappy/xz/zlib/bzip2/LZ4 compression) and describe_riegeli_file.
# With QNX's change (patches/0001, from github.com/qnx-ports/riegeli, tag
# qnx-e54e4300-r0; upstream commit e54e4300 of 2023-12-12 is what QNX's port
# is based on, and it builds against this tree's abseil 20240722).
VERSION=0_git20231212; GIT=https://github.com/google/riegeli.git; COMMIT=e54e4300f61cc113e5da816952d20ae070fe7219
DEPS="abseil protobuf brotli zstd snappy xz zlib bzip2 lz4 highwayhash"
BUILDSYS=custom
# Upstream builds only with Bazel. files/CMakeLists.txt builds the library
# from the same sources (all of riegeli/ except the TensorFlow ops). The
# protobuf code is generated with the host protoc the protobuf recipe built
# (29.3, the version its generated code must match). Left out: ReaderCFile()/
# WriterCFile() (a Reader/Writer as a stdio FILE*), made with glibc's
# fopencookie(), which QNX's C library does not have. snappy is built without
# RTTI, so riegeli's snappy stream adapters are too. patches/0002: zlib's and
# LZ4's headers by their installed names (riegeli named them by Bazel paths);
# patches/0003: protobuf 29's error-collector interface (RecordError).
_host_protoc() { find "$BUILD_DIR/host/protobuf" -maxdepth 1 -name 'protoc*' -type f -perm -u+x | head -1; }
do_configure() {
  cmake -S "$RECIPE_DIR/riegeli/files" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DRIEGELI_SOURCE_DIR="$S" -DPROTOC="$(_host_protoc)" \
    -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl" -Dprotobuf_DIR="$STAGE$PREFIX/lib/cmake/protobuf"
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
