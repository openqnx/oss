# GNU debugger, running on the target. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=14.2
TARBALL=https://ftp.gnu.org/gnu/gdb/gdb-14.2.tar.xz
SHA256=2d4dd8061d8ded12b6c63f55e45344881e8226105f4d2a9b234040efa5ce7772
# Also linked at run time (found by scripts/gen-dependency-index): libiconv xxhash xz zstd
DEPS="gmp mpfr ncurses readline expat zlib libiconv xxhash xz zstd"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --with-system-readline --without-python --without-guile --disable-sim --with-expat --with-system-zlib --disable-werror )
# With Automake's silent rules, a bfd rule passes its "@echo" prefix to
# depcomp as the command ("@echo: command not found"); build verbosely.
do_build() { make -C "$B" -j"$(nproc)" V=1; }
# The QNX (nto) target code in the aports patch is guarded by __QNXTARGET__,
# which aports' APKBUILD defines on the command line.
CFLAGS_APPEND="-D__QNXTARGET__"
# Install gdb and gdbserver only. The top-level install also installs the
# libbfd, libopcodes and libctf that gdb builds for itself -- static archives
# and headers that are the binutils component's, which then had two owners.
do_install() { make -C "$B" DESTDIR="$STAGE" install-gdb maybe-install-gdbserver; }
# gdb's own include/ (md5.h, sha1.h) ahead of the staging tree's: libmd's
# staged md5.h otherwise hides include/md5.h and libiberty's md5.c fails on
# an undefined struct md5_ctx -- as for binutils.
CONF_ENV=( CPPFLAGS="-I$SRC_DIR/$NAME/include -D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include"
  CFLAGS="-I$SRC_DIR/$NAME/include -D__QNXTARGET__ -D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" )
