VERSION=1.18.2; GIT=https://gitlab.freedesktop.org/cairo/cairo.git; REF=1.18.2
# Also linked at run time (found by scripts/gen-dependency-index): lzo
DEPS="pixman freetype fontconfig libpng zlib glib libx11 libxext libxrender lzo"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=disabled -Dgtk_doc=false
                   -Dfreetype=enabled -Dfontconfig=enabled -Dpng=enabled
                   -Dzlib=enabled -Dglib=enabled
                   # The Xlib surface: originally disabled on the grounds that
                   # QNX has no window system.  The X11 client stack is staged
                   # now, and GTK3's X11 backend -- which Thunar 4.20 requires --
                   # draws through cairo-xlib.  patches/0001 lets it build
                   # without MIT-SHM, which needs SysV shm that QNX lacks.
                   # The XCB surface stays off: nothing here uses it, and its
                   # SHM path (cairo-xcb-shm.c) has the same SysV dependency.
                   -Dxlib=enabled -Dxcb=disabled -Dxlib-xcb=disabled -Dquartz=disabled
                   -Dtee=enabled -Dsymbol-lookup=disabled -Dspectre=disabled )
