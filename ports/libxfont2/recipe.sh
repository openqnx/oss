# libxfont2: X font rendering (PCF, BDF, FreeType fonts) as X servers and Xwayland use it.
# --disable-fc: the xfs font-server client (fslibos.h) needs NOFILES_MAX and
# fd_set internals QNX does not have; font servers are a relic, local fonts work.
VERSION=2.0.10
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libXfont2-2.0.10.tar.xz
SHA256=cef1f4ad9cd69f439bb1ac484bdc8f9d7f8e70215b92d8ca86b96e31513e5f9c
DEPS="freetype zlib fontenc xorgproto xtrans"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-fc --without-xmlto --without-fop --disable-devel-docs )
