# libmd: message digest functions from BSD systems (MD5, SHA-1, SHA-2, RIPEMD-160; libbsd's companion).
VERSION=1.1.0
TARBALL=https://libbsd.freedesktop.org/releases/libmd-1.1.0.tar.xz
SHA256=1bd6aa42275313af3141c7cf2e5b964e8b1fd488025caf2f971f43b00776b332
DEPS=""
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
