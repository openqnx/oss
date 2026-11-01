VERSION=10.5.2
TARBALL=https://github.com/NetworkConfiguration/dhcpcd/releases/download/v10.5.2/dhcpcd-10.5.2.tar.xz
SHA256=3e476657fdb6eeb38b277da3a48d0ac0113ecce5858ebdcddff2b629faed52b4
BUILDSYS=custom
# dhcpcd's configure is hand-written (not autoconf) and builds in its source
# directory, so the tree is copied into the build directory first.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
  ./configure --host="$QNX_TRIPLE" --os=qnx --prefix="$PREFIX" \
     --sysconfdir="$PREFIX/etc" --libexecdir="$PREFIX/libexec" \
     --dbdir=/var/db/dhcpcd --runstatedir=/var/run \
     --disable-privsep --without-openssl --with-hooks=; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
