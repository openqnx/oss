# RocksDB: embeddable persistent key-value store (LSM tree) for fast storage,
# with column families, transactions, snapshots, iterators, compaction and
# backups; plus the ldb and sst_dump tools.
VERSION=11.8.1; GIT=https://github.com/facebook/rocksdb.git; REF=v11.8.1
# Compression: LZ4, zlib and ZSTD. Not snappy: the snappy component is built
# as upstream builds it, without RTTI, and RocksDB (built with RTTI, which
# its users need to derive from its classes) subclasses snappy::Sink, whose
# typeinfo then does not exist ("undefined reference to typeinfo for
# snappy::Sink").
DEPS="lz4 zlib zstd"
CMAKE_ARGS_EXTRA=( -DROCKSDB_BUILD_SHARED=ON -DWITH_GFLAGS=OFF -DWITH_TESTS=OFF -DWITH_ALL_TESTS=OFF
                   -DWITH_BENCHMARK_TOOLS=OFF -DWITH_TRACE_TOOLS=OFF -DWITH_CORE_TOOLS=ON -DWITH_TOOLS=OFF
                   -DWITH_SNAPPY=OFF -DWITH_LZ4=ON -DWITH_ZLIB=ON -DWITH_ZSTD=ON -DWITH_BZ2=OFF
                   -DWITH_LIBURING=OFF -DWITH_JEMALLOC=OFF -DWITH_NUMA=OFF -DWITH_JNI=OFF
                   -DPORTABLE=1 -DFAIL_ON_WARNINGS=OFF -DUSE_RTTI=1 -DROCKSDB_NO_FAST_LINKER=ON )
# The core tools (ldb, sst_dump) are built but have no install rule.
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  install -Dm755 "$B/tools/ldb" "$STAGE$PREFIX/bin/ldb"
  install -Dm755 "$B/tools/sst_dump" "$STAGE$PREFIX/bin/sst_dump"
}
