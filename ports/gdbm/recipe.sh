VERSION=1.24; TARBALL=https://ftp.gnu.org/gnu/gdbm/gdbm-1.24.tar.gz
SHA256=695e9827fdf763513f133910bc7e6cfdb9187943a4fec943e57449723d2b8dbf
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-nls
                  --without-readline --enable-libgdbm-compat )
