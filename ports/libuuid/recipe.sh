# libuuid from util-linux, the maintained one: the standalone libuuid 1.0.3
# on SourceForge is a 2014 snapshot of this same code that has had no fixes
# since. Only the library is built (--disable-all-programs --enable-libuuid):
# libuuid.so.1, <uuid/uuid.h> and uuid.pc, the same ABI as on Linux.
VERSION=2.42.4
TARBALL=https://mirrors.edge.kernel.org/pub/linux/utils/util-linux/v2.42/util-linux-2.42.4.tar.xz
SHA256=fbd62a100ab7bb8746ba0661255c3c48185b1e9021507c624da01fbc696330ec
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-all-programs --enable-libuuid --enable-shared --disable-static
  --disable-nls --disable-bash-completion --disable-asciidoc --disable-poman --without-python
  --without-systemd --without-udev --without-ncurses --without-ncursesw --without-readline
  --without-libmagic --without-cap-ng --without-btrfs --without-selinux --without-audit
  --without-user --without-econf --disable-liblastlog2 )
# util-linux installs terminal-colors.d(5) whatever is enabled; it describes
# programs this package does not have.
do_install() {
  make -C "$B" DESTDIR="$STAGE" install && rm -f "$STAGE$PREFIX/share/man/man5/terminal-colors.d.5" &&
  rmdir "$STAGE$PREFIX/share/man/man5" 2>/dev/null; true
}
