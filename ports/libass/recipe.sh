VERSION=0.17.5; GIT=https://github.com/libass/libass.git; REF=0.17.5
DEPS="freetype fribidi harfbuzz fontconfig"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-require-system-font-provider )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
