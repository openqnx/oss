VERSION=3.3.0
TARBALL=https://download.samba.org/pub/rsync/src/rsync-3.3.0.tar.gz
SHA256=7399e9a6708c32d678a72a63219e96f23be0be2336e50fd1348498d07041df90
DEPS="zstd lz4 xxhash openssl"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-md2man --disable-debug --with-included-popt --with-included-zlib )
# makedev() takes three arguments on QNX (node, major, minor). configure finds
# that with a run test, which cannot execute when cross compiling and then
# defaults to "no"; give it the answer.
CONF_ENV=( rsync_cv_MAKEDEV_TAKES_3_ARGS=yes )
