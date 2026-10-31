VERSION=1.1.8; GIT=https://gitlab.freedesktop.org/xorg/lib/libfontenc.git; REF=libfontenc-1.1.8
DEPS="xorgproto zlib"
BUILDSYS=autotools
# XORG_FONTROOTDIR asks pkg-config for fontutil's fontrootdir, and the
# cross pkg-config prefixes variables with the sysroot: the build machine's
# stage/<arch>/... path was compiled in as the encodings directory, so
# FontEncFind() found nothing on a target. Give the target path.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-fontrootdir="$PREFIX/share/fonts/X11" )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
