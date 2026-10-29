VERSION=1.3.7
TARBALL=https://downloads.xvid.com/downloads/xvidcore-1.3.7.tar.gz
SHA256=abbdcbd39555691dd1c9b4d08f0a031376a3b211652c0d8b3b8aa9be1303ce2d
SUBDIR=build/generic
BUILDSYS=custom
# xvid ships its Makefile in the source directory and configure only writes
# platform.inc beside it, so there is nothing to build out-of-tree. Configure and
# build in place, the way its own bootstrap.sh expects.
do_configure() {
  cd "$S" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
    ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" --disable-assembly
}
do_build()   { make -C "$S" -j"$(nproc)"; }
do_install() { make -C "$S" install DESTDIR="$STAGE"; }
