VERSION=2.1-3
TARBALL=https://downloads.sourceforge.net/project/cunit/CUnit/2.1-3/CUnit-2.1-3.tar.bz2
SHA256=f5b29137f845bb08b77ec60584fdb728b4e58f1023e6f249a464efa49a40f214
BUILDSYS=custom
# The 2.1-3 tarball ships configure.in without configure, and a source tree
# autoconf considers "already configured" -- an out-of-tree build is refused.
# Build in a copy of the tree instead.
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" \
     --enable-shared --disable-static --disable-console
}
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
