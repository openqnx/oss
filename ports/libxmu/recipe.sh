# libxmu: X miscellaneous utilities (atoms, window-manager helpers, Xt converters).
VERSION=1.3.1
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libXmu-1.3.1.tar.xz
SHA256=81a99e94c4501e81c427cbaa4a11748b584933e94b7a156830c3621256857bc4
DEPS="libx11 libxext libxt xorgproto"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-xmlto --without-fop --disable-specs )
