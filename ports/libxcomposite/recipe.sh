VERSION=0.4.6; GIT=https://gitlab.freedesktop.org/xorg/lib/libxcomposite.git; REF=libXcomposite-0.4.6
DEPS="libx11 libxfixes xorgproto"
BUILDSYS=autotools
# Needed by GTK3's X11 backend (HAVE_XCOMPOSITE), which Thunar requires.
# --disable-malloc0returnsnull: see libxfixes -- QNX documents that malloc(0)
# returns a valid pointer, not NULL.
CONF_ARGS_EXTRA=( --disable-malloc0returnsnull --enable-shared --disable-static --without-xmlto --without-fop --disable-specs )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
