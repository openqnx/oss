# libmd: message digest functions from BSD systems (MD5, SHA-1, SHA-2, RIPEMD-160; libbsd's companion).
VERSION=1.3.0
TARBALL=https://libbsd.freedesktop.org/releases/libmd-1.3.0.tar.xz
SHA256=fc0f1eb6b6766470326f2c014693809190e67dba84274a6fbae9d4912d066706
DEPS=""
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
