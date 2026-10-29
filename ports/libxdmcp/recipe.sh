VERSION=1.1.5; GIT=https://gitlab.freedesktop.org/xorg/lib/libxdmcp.git; REF=libXdmcp-1.1.5
DEPS="xorgproto"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-xmlto --without-fop )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
