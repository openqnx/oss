VERSION=1.5.2; GIT=https://gitlab.freedesktop.org/xorg/lib/libxtrans.git; REF=xtrans-1.5.2
BUILDSYS=autotools
# Header-only: it is #included by libX11 and libxcb rather than linked.
CONF_ARGS_EXTRA=( --disable-docs )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
