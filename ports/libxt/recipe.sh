# libxt: the X Toolkit Intrinsics (Xt), the base of Athena/Motif-era X programs and of libXmu.
VERSION=1.3.1
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libXt-1.3.1.tar.xz
SHA256=e0a774b33324f4d4c05b199ea45050f87206586d81655f8bef4dba434d931288
DEPS="libx11 libsm libice xorgproto"
BUILDSYS=autotools
# malloc(0) returns a valid pointer on QNX (see libxext).
CONF_ARGS_EXTRA=( --disable-malloc0returnsnull --enable-shared --disable-static --without-xmlto --without-fop --disable-specs )
# util/makestrs runs on the build machine while building: its own compiler
# and flags, not the QNX include path the driver gives the cross compiler.
CONF_ENV=( CC_FOR_BUILD=gcc CPPFLAGS_FOR_BUILD= CFLAGS_FOR_BUILD=-O2 LDFLAGS_FOR_BUILD= )
