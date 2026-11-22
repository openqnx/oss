VERSION=3.0.5; GIT=https://github.com/sekrit-twc/zimg.git; REF=release-3.0.5
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
do_patch() { cd "$SRCROOT" && git submodule update --init --recursive >/dev/null 2>&1; autoreconf -fi; }
