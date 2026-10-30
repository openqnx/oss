# libxfont2: X font rendering (PCF, BDF, FreeType fonts) as X servers and Xwayland use it.
# --disable-fc: the xfs font-server client (fslibos.h) needs NOFILES_MAX and
# fd_set internals QNX does not have; font servers are a relic, local fonts work.
VERSION=2.0.7
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libXfont2-2.0.7.tar.xz
SHA256=8b7b82fdeba48769b69433e8e3fbb984a5f6bf368b0d5f47abeec49de3e58efb
DEPS="freetype zlib fontenc xorgproto xtrans"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-fc --without-xmlto --without-fop --disable-devel-docs )
