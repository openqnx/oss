VERSION=0.17.3; GIT=https://github.com/libass/libass.git; REF=0.17.3
DEPS="freetype fribidi harfbuzz fontconfig"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-require-system-font-provider )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
