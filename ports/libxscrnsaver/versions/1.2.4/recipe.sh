VERSION=1.2.4; GIT=https://gitlab.freedesktop.org/xorg/lib/libxscrnsaver.git; REF=libXScrnSaver-1.2.4
DEPS="libx11 libxext xorgproto"
BUILDSYS=autotools
# The MIT-SCREEN-SAVER extension client library.  Here because SDL3's X11
# backend lists xscrnsaver among its hard pkg-config requirements -- it uses
# it to inhibit the screensaver while a window has focus.
CONF_ARGS_EXTRA=( --disable-malloc0returnsnull --enable-shared --disable-static --without-xmlto --without-fop --disable-specs )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
