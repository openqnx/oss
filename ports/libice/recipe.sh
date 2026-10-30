# libice: the X Inter-Client Exchange protocol (the transport of X session management).
VERSION=1.1.2
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libICE-1.1.2.tar.xz
SHA256=974e4ed414225eb3c716985df9709f4da8d22a67a2890066bc6dfc89ad298625
DEPS="xorgproto xtrans"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-xmlto --without-fop --disable-docs --disable-specs )
