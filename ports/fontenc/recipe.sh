VERSION=1.1.8; GIT=https://gitlab.freedesktop.org/xorg/lib/libfontenc.git; REF=libfontenc-1.1.8
DEPS="xorgproto zlib"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
