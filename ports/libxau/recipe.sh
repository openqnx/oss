VERSION=1.0.12; GIT=https://gitlab.freedesktop.org/xorg/lib/libxau.git; REF=libXau-1.0.12
DEPS="xorgproto"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
