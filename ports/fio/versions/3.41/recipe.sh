# fio: the flexible I/O tester -- benchmarks and verifies storage with job
# files (sequential/random reads and writes, many I/O engines, data
# verification). Version as in QNX's own package list.
VERSION=3.41; GIT=https://github.com/axboe/fio.git; REF=fio-3.41
# zlib: compressed I/O logs (log_compression) and the zlib buffer
# compression test. Nothing else from staging: --disable-http leaves out the
# http (WebDAV/S3/Swift) engine, which would link curl and OpenSSL.
DEPS="zlib"
# The helper scripts (fio2gnuplot, fiologparser*.py, fio_jsonplus_clat2csv,
# ...) are Python and genfio is bash: they need python3 / bash on the target,
# which fio itself does not; they are not declared as dependencies.
# fio has its own configure (not Autoconf) and already knows QNX
# (os/os-qnx.h): it detects the target from the compiler (__QNX__), so it is
# given qcc and the target's flags; --disable-native keeps -march=native (the
# build machine's CPU) out of a cross build.
# --disable-shm: QNX has no System V shared memory (no <sys/shm.h>, no
# shmget), which fio uses to share job state between forked job processes.
# Without it fio runs every job as a thread (thread=1 is the default then)
# and has no client/server mode (--server, --client).
BUILDSYS=custom
do_configure() {
  # fio's configure and Makefile work in the source tree only: build in a
  # copy of it, so src/fio stays clean (scripts/mkpatch diffs it). The
  # staging include directory comes first: zlib.h must be the zlib port's,
  # the library fio links.
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" && rm -rf "$B/.git" || return 1
  cd "$B" && ./configure --prefix="$PREFIX" \
    --cc="qcc -V$QCC_TARGET" --extra-cflags="-D_QNX_SOURCE -I$STAGE_SYSROOT$PREFIX/include $CFLAGS" \
    --disable-native --disable-shm --disable-http 2>&1 | tail -n 80
}
do_build() { make -C "$B" -j"$(nproc)" V=1; }
do_install() {
  make -C "$B" DESTDIR="$STAGE" prefix="$PREFIX" install || return 1
  # genfio starts with #!/bin/bash; a QNX target's bash is the bash port's.
  sed -i "1s|^#!/bin/bash|#!$PREFIX/bin/bash|" "$STAGE$PREFIX/bin/genfio"
}
