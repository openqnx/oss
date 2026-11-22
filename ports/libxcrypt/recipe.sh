# libxcrypt (modern crypt() hashes). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=4.5.2
TARBALL=https://github.com/besser82/libxcrypt/releases/download/v4.5.2/libxcrypt-4.5.2.tar.xz
SHA256=71513a31c01a428bccd5367a32fd95f115d6dac50fb5b60c779d5c7942aec071
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-werror --enable-hashes=strong,glibc --enable-obsolete-api=no )
