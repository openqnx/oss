VERSION=1.6.0; GIT=https://gitlab.freedesktop.org/xorg/lib/libxtrans.git; REF=xtrans-1.6.0
BUILDSYS=autotools
# Header-only: it is #included by libX11 and libxcb rather than linked.
CONF_ARGS_EXTRA=( --disable-docs )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
