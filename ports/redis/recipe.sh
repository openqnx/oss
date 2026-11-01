# Redis: in-memory data structure server (redis-server, redis-sentinel) and
# its command-line client, benchmark and RDB/AOF checkers.
# The core server only: the Redis 8 modules (JSON, search, time series,
# probabilistic) are separate repositories that scripts/build.sh builds when
# cloned under modules/, and they need Rust and other toolchains; they are
# not part of this port. TLS is built in, against the staged OpenSSL.
VERSION=8.10.3; GIT=https://github.com/redis/redis.git; REF=8.10.3
DEPS="openssl"
BUILDSYS=custom
# The Makefile is used directly from src/ (the top-level one goes through
# scripts/build.sh and its module detection). Notes on the variables:
#  - uname_S=QNX: the Makefile asks the build machine's uname, which says
#    Linux; QNX takes its generic "other OSes" branch (-ldl -lrt -pthread are
#    the empty compatibility archives, hazard 3).
#  - CC is the cross-GCC driver, not qcc: xxhash's multiconf.make compiles
#    with -MMD -MP -MF and qcc read that as a second input ("input file ... is
#    the same as output file", hazard 18/21).
#  - MALLOC=libc: the vendored jemalloc is the Linux default; QNX's
#    allocator is used instead (zmalloc keeps its own size header then).
#  - OPTIMIZATION=-O2: -O3 turns on -flto=auto, which the cross toolchain's
#    ar/ranlib do not take part in for the static deps.
#  - -lsocket -lepoll: QNX keeps sockets and epoll out of libc (hazards 2, 38).
#  - an 8 MiB main-thread stack: QNX's default is small (hazard 119), and the
#    server runs Lua scripts and its RDB/AOF loaders on the main thread.
_redis_make() {
  make -C "$B/src" -j"$(nproc)" uname_S=QNX uname_M="$QNX_CPU" \
    CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -D_QNX_SOURCE" \
    AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" RANLIB="$QNX_HOST/usr/bin/nto${QNX_CPU}-ranlib" \
    MALLOC=libc BUILD_TLS=yes USE_SYSTEMD=no OPTIMIZATION=-O2 \
    OPENSSL_PREFIX="$STAGE_SYSROOT$PREFIX" \
    REDIS_LDFLAGS="-L$STAGE_SYSROOT$PREFIX/lib -lsocket -lepoll -Wl,-z,stack-size=8388608" "$@"
}
# Built in a copy of the tree: the Makefiles write their objects (and the
# dependency libraries) next to the sources. The programs are named as
# targets: `all` (and `install`, which depends on it) also builds the
# test-suite modules under tests/modules, which link with a bare `ld` -- the
# build machine's -- and fail on QNX objects. They are not installed anyway.
do_build() { rm -rf "$B" && cp -a "$S" "$B" &&
  _redis_make redis-server redis-sentinel redis-cli redis-benchmark redis-check-rdb redis-check-aof; }
# What `make install` does, without its dependency on `all`: the three
# programs, and the checkers and sentinel as links to redis-server (it
# dispatches on argv[0]). The sample configurations go to etc/redis.
do_install() {
  local bin="$STAGE$PREFIX/bin" p
  mkdir -p "$bin" || return 1
  for p in redis-server redis-cli redis-benchmark; do install -m755 "$B/src/$p" "$bin/" || return 1; done
  for p in redis-check-rdb redis-check-aof redis-sentinel; do ln -sf redis-server "$bin/$p" || return 1; done
  install -Dm644 "$B/redis.conf" "$STAGE$PREFIX/etc/redis/redis.conf" &&
  install -Dm644 "$B/sentinel.conf" "$STAGE$PREFIX/etc/redis/sentinel.conf"
}
