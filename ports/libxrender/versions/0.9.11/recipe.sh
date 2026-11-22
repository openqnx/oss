VERSION=0.9.11; GIT=https://gitlab.freedesktop.org/xorg/lib/libxrender.git; REF=libXrender-0.9.11
DEPS="libx11 xorgproto"
BUILDSYS=autotools
# malloc(0) is a run-time test, which cross configure cannot perform. QNX
# documents it explicitly: "If size is zero, then malloc() returns a valid
# pointer with a size of zero" -- so it does not return NULL, and Xlib does
# not need its add-a-byte workaround.
CONF_ARGS_EXTRA=( --disable-malloc0returnsnull --enable-shared --disable-static --without-xmlto --without-fop --disable-specs )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
