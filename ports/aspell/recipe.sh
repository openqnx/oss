VERSION=0.60.8.1; GIT=https://github.com/GNUAspell/aspell.git; REF=rel-0.60.8.1
DEPS="ncurses"   # configure links -lncurses(w); undeclared, it found the SDP's
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-nls
                  --enable-compile-in-filters )
do_patch() { cd "$SRCROOT" && ./autogen >/dev/null 2>&1 || autoreconf -fi; }
