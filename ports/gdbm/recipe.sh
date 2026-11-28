VERSION=1.26
TARBALL=https://ftp.gnu.org/gnu/gdbm/gdbm-1.26.tar.gz
SHA256=6a24504a14de4a744103dcb936be976df6fbe88ccff26065e54c1c47946f4a5e
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-nls
                  --without-readline --enable-libgdbm-compat )
