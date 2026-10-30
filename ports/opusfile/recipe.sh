# opusfile: decoding and seeking in .opus files (Ogg Opus), from files or HTTP(S) streams.
VERSION=0.12
TARBALL=https://downloads.xiph.org/releases/opus/opusfile-0.12.tar.gz
SHA256=118d8601c12dd6a44f52423e68ca9083cc9f2bfe72da7a8c1acb22a80ae3550b
DEPS="opus libogg openssl"
BUILDSYS=autotools
# The examples are not installed; seeking_example.c also redefines a stdlib.h
# keyword QNX's header uses.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-examples --disable-doc )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
