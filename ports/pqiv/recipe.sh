# pqiv, a minimalist image viewer (GTK3).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.13.3
TARBALL=https://github.com/phillipberndt/pqiv/archive/refs/tags/2.13.3.tar.gz
SHA256=f0ffaa33e93299b38058c507da2945976a4b350c92cf1c4b3649586444395dfd
DEPS="glib gtk3 gdk-pixbuf cairo"
BUILDSYS=custom
# pqiv's configure is hand-written; it takes the compiler and the backends.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" PKG_CONFIG=pkg-config \
  ./configure --prefix="$PREFIX" --backends-build=static --backends=gdkpixbuf \
     ; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
