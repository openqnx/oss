# wv: the library that reads Microsoft Word 2000/97/95/6 documents (the .doc
# format), with converters (wvHtml, wvText, wvRTF, ...). AbiWord's .doc import
# is built on it.
VERSION=1.2.9
# abisource.com, the upstream site, no longer serves it; Debian keeps the
# unchanged release tarball.
TARBALL=https://deb.debian.org/debian/pool/main/w/wv/wv_1.2.9.orig.tar.gz
SHA256=4c730d3b325c0785450dd3a043eeb53e1518598c4f41f155558385dd2635c19d
DEPS="glib libgsf zlib libpng"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-libwmf )
# patches/0001 changes configure.ac and configure: keep the generated files
# newer than their sources, or make reruns autoconf/automake.
do_patch() {
  cd "$SRCROOT" && touch aclocal.m4 && sleep 1 && touch configure config.h.in && \
  find . -name 'GNUmakefile.in' -exec touch {} +
}
