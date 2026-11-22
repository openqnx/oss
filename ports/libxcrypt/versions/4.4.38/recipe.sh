# libxcrypt (modern crypt() hashes). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=4.4.38
TARBALL=https://github.com/besser82/libxcrypt/releases/download/v4.4.38/libxcrypt-4.4.38.tar.xz
SHA256=80304b9c306ea799327f01d9a7549bdb28317789182631f1b54f4511b4206dd6
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-werror --enable-hashes=strong,glibc --enable-obsolete-api=no )
