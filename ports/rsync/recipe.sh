VERSION=3.5.2
TARBALL=https://download.samba.org/pub/rsync/src/rsync-3.5.2.tar.gz
SHA256=a10d591825251e070f74c98df3daad195def4ed81b4a2de16169026aea33f6c8
DEPS="zstd lz4 xxhash openssl"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-md2man --disable-debug --with-included-popt --with-included-zlib )
# makedev() takes three arguments on QNX (node, major, minor). configure finds
# that with a run test, which cannot execute when cross compiling and then
# defaults to "no"; give it the answer.
CONF_ENV=( rsync_cv_MAKEDEV_TAKES_3_ARGS=yes )
