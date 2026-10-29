VERSION=1.0.20-git-20250309; GIT=https://github.com/akopytov/sysbench.git; COMMIT=3ceba0b1e115f8c50d1d045a4574d8ed643bd497
DEPS="luajit"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --without-mysql --without-pgsql --with-system-luajit )
# sysbench builds its bundled ConcurrencyKit with ConcurrencyKit's own
# configure, which recognises compilers by name and rejects "qcc". The GCC
# driver qcc runs is named like a GCC and works directly.
CONF_ENV=( QNXENV=true CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" LUAJIT_CFLAGS="-I$STAGE$PREFIX/include/luajit-2.1" LUAJIT_LIBS="-L$STAGE$PREFIX/lib -lluajit-5.1" )
# The QNX patch makes ConcurrencyKit's configure (run during "make") skip its
# run-time compiler probe when QNXENV=true is set -- a cross build cannot run
# what it compiles.
do_build() { QNXENV=true make -C "$B" -j"$(nproc)"; }
# The patches change tests/Makefile.am; regenerate the build files.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
