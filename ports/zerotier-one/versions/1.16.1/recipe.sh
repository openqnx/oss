# ZeroTier One from upstream 1.16.1 with qnx-ports' QNX support (make-qnx.mk,
# a /dev/tap EthernetTap, BSD routing paths); see patches/0001.
VERSION=1.16.1; GIT=https://github.com/zerotier/ZeroTierOne.git; REF=1.16.1
BUILDSYS=custom
# The Makefile builds in the source tree and picks make-qnx.mk from OSTYPE,
# which it otherwise takes from the build machine's uname.
# CC_MACH: make-qnx.mk derives the architecture from `$(CC) -dumpmachine`,
# which qcc (a driver, not GCC) does not answer.
# QNX_RELEASE: the target's release for OS_STRING, not the build machine's (0002).
# STRIP: the QNX cross strip, not the build machine's.
# The bundled miniupnpc/libnatpmp are used (make-qnx.mk); the Rust-based SSO
# support and the network controller (non-free) are not built.
_zt_make() {
  make -C "$B" OSTYPE=QNX CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
    CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" CC_MACH="$QNX_CPU" QNX_RELEASE=8.0.0 \
    STRIP="$QNX_HOST/usr/bin/$QNX_TRIPLE-strip" "$@"
}
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _zt_make -j"$(nproc)" one; }
# zerotier-cli and zerotier-idtool are the same program under other names
# (it looks at argv[0]), installed as symlinks as upstream's install does.
do_install() {
  install -d "$STAGE$PREFIX/sbin" "$STAGE$PREFIX/share/man/man1" "$STAGE$PREFIX/share/man/man8"
  install -m 755 "$B/zerotier-one" "$STAGE$PREFIX/sbin/zerotier-one"
  ln -sf zerotier-one "$STAGE$PREFIX/sbin/zerotier-cli"
  ln -sf zerotier-one "$STAGE$PREFIX/sbin/zerotier-idtool"
  install -m 644 "$B"/doc/zerotier-cli.1 "$B"/doc/zerotier-idtool.1 "$STAGE$PREFIX/share/man/man1/"
  install -m 644 "$B"/doc/zerotier-one.8 "$STAGE$PREFIX/share/man/man8/"
}
