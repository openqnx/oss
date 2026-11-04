# tdb: Samba's "trivial database" -- a small, fast key/value store with
# transactions and multi-process locking, used by Samba and others -- with
# tdbtool, tdbdump, tdbrestore and tdbbackup. Standalone build of the
# Samba project's release (the copy samba bundles is built with samba).
VERSION=1.4.15
TARBALL=https://download.samba.org/pub/tdb/tdb-1.4.15.tar.gz
SHA256=fba09d8df1f1b9072aeae8e78b2bd43c5afef20b2f6deefa633aa14a377a8dd2
BUILDSYS=custom
# waf (Samba's buildtools) in cross mode: every configure question that
# needs a test program run is answered from ports/tdb/cross-answers-<arch>.txt,
# recorded once on QNX targets (see the comments there). waf builds in the
# source tree (bin/), so it runs in a copy. No Python module (the target
# Python bindings would need the cross Python's headers and ABI).
# --disable-tdb-mutex-locking: tdb's robust-mutex mode keeps process-shared
# mutexes in a mapped area of the database file; on QNX Samba's first
# opens of a database failed there ("tdb_mutex_init failed ...: File
# exists"), so tdb locks with fcntl() as it does wherever mutexes are off.
_tdb_waf_env() { env CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS="-O2 -I$STAGE_SYSROOT$PREFIX/include" \
  LDFLAGS="-L$STAGE_SYSROOT$PREFIX/lib -lsocket" PYTHONHASHSEED=1 "$@"; }
do_build() {
  rm -rf "$B" && cp -a "$S" "$B" && cd "$B" &&
  cp "$WS/ports/tdb/cross-answers-$QNX_ARCH.txt" "$B/cross-answers.txt" &&
  _tdb_waf_env ./configure --prefix="$PREFIX" --disable-python --disable-rpath --disable-rpath-install --disable-tdb-mutex-locking \
    --cross-compile --cross-answers="$B/cross-answers.txt" --hostcc=gcc &&
  _tdb_waf_env make -j"$(nproc)"
}
do_install() { cd "$B" && _tdb_waf_env make install DESTDIR="$STAGE"; }
