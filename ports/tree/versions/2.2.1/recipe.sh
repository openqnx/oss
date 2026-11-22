VERSION=2.2.1; GIT=https://gitlab.com/OldManProgrammer/unix-tree.git; REF=2.2.1
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64" LDFLAGS=""; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/tree" "$STAGE$PREFIX/bin/"; }
