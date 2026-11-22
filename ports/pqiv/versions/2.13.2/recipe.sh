# pqiv, a minimalist image viewer (GTK3).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.13.2
TARBALL=https://github.com/phillipberndt/pqiv/archive/refs/tags/2.13.2.tar.gz
SHA256=154cbbe0a62be12cee23b0a46a86b2305d8128fd19924308ad5e9d22fa5ad4f7
DEPS="glib gtk3 gdk-pixbuf cairo"
BUILDSYS=custom
# pqiv's configure is hand-written; it takes the compiler and the backends.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" PKG_CONFIG=pkg-config \
  ./configure --prefix="$PREFIX" --backends-build=static --backends=gdkpixbuf \
     ; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
