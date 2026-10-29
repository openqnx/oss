VERSION=1.8.10; GIT=https://gitlab.freedesktop.org/xorg/lib/libx11.git; REF=libX11-1.8.10
DEPS="libxcb xorgproto xtrans"
BUILDSYS=autotools
# makekeys is a build-machine tool libX11 compiles and runs during the build;
# makekeys is a build-machine tool libX11 compiles and runs during the build.
# malloc(0) is a run-time test, which cross configure cannot perform. QNX
# documents it explicitly: "If size is zero, then malloc() returns a valid
# pointer with a size of zero" -- so it does not return NULL, and Xlib does
# not need its add-a-byte workaround.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-xmlto --without-fop
                  --disable-specs --disable-malloc0returnsnull
                  --with-keysymdefdir="$STAGE$PREFIX/include/X11" )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
