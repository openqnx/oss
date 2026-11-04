# Valkey: the open source key/value data store forked from Redis 7.2
# (valkey-server, valkey-sentinel) with its command-line client, benchmark
# and RDB/AOF checkers. TLS is built in, against the staged OpenSSL.
VERSION=9.1.3; GIT=https://github.com/valkey-io/valkey.git; REF=9.1.3
DEPS="openssl"
BUILDSYS=custom
# Built like ports/redis (the same Makefile lineage). Notes on the variables:
#  - uname_S=QNX: the Makefile asks the build machine's uname, which says
#    Linux; QNX takes its generic "other OSes" branch.
#  - CC is the cross-GCC driver, not qcc: the vendored dependencies compile
#    with -MMD -MF, which qcc misreads (hazards 18/21). The bare driver lacks
#    the __unix__/__unix that qcc predefines, and the vendored libvalkey
#    picks its pthread_once() path by __unix__ (#error otherwise).
#  - MALLOC=libc: the vendored jemalloc is the Linux default; QNX's
#    allocator is used instead (zmalloc keeps its own size header then).
#  - OPTIMIZATION=-O2: -O3 turns on -flto=auto, which the cross toolchain's
#    ar/ranlib do not take part in for the static deps.
#  - -lsocket -lepoll: QNX keeps sockets and epoll out of libc (hazards 2, 38).
#  - an 8 MiB main-thread stack: QNX's default is small (hazard 119), and the
#    server runs Lua scripts and its RDB/AOF loaders on the main thread.
_valkey_make() {
  make -C "$B/src" -j"$(nproc)" uname_S=QNX uname_M="$QNX_CPU" \
    CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -D_QNX_SOURCE -D__unix__ -D__unix" \
    AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" RANLIB="$QNX_HOST/usr/bin/nto${QNX_CPU}-ranlib" \
    MALLOC=libc USE_JEMALLOC=no BUILD_TLS=yes USE_SYSTEMD=no OPTIMIZATION=-O2 \
    OPENSSL_PREFIX="$STAGE_SYSROOT$PREFIX" \
    SERVER_LDFLAGS="-L$STAGE_SYSROOT$PREFIX/lib -lsocket -lepoll -Wl,-z,stack-size=8388608" "$@"
}
# Built in a copy of the tree (the Makefiles write objects next to the
# sources). The programs are named as targets: `all` also builds test modules
# that link with the build machine's bare `ld`.
do_build() { rm -rf "$B" && cp -a "$S" "$B" &&
  _valkey_make valkey-server valkey-sentinel valkey-cli valkey-benchmark valkey-check-rdb valkey-check-aof; }
# What `make install` does, without its dependency on `all`: the three
# programs, and the checkers and sentinel as links to valkey-server (it
# dispatches on argv[0]). The sample configurations go to etc/valkey.
do_install() {
  local bin="$STAGE$PREFIX/bin" p
  mkdir -p "$bin" || return 1
  for p in valkey-server valkey-cli valkey-benchmark; do install -m755 "$B/src/$p" "$bin/" || return 1; done
  for p in valkey-check-rdb valkey-check-aof valkey-sentinel; do ln -sf valkey-server "$bin/$p" || return 1; done
  install -Dm644 "$B/valkey.conf" "$STAGE$PREFIX/etc/valkey/valkey.conf" &&
  install -Dm644 "$B/sentinel.conf" "$STAGE$PREFIX/etc/valkey/sentinel.conf"
}
