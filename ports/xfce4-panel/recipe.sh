VERSION=4.20.8
# Release tarball, as for every Xfce package here -- see README hazard 74.
TARBALL=https://archive.xfce.org/src/xfce/xfce4-panel/4.20/xfce4-panel-4.20.8.tar.bz2
SHA256=d69cb1f377953aeb1fb9bdbcef12c246bea66586e3f2868f3b758e0e8ce3d3fe
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell wayland wayland-protocols"
BUILDSYS=autotools
# Wayland only, matching libxfce4windowing, which was built --disable-x11: the
# panel's X11 half needs libwnck, and libwnck's window tracking is X11 through
# and through.  gtk-layer-shell is what lets the panel anchor itself to a screen
# edge under a wlroots-style compositor.
# glib-compile-resources runs at build time: the build host's, not the staged QNX
# one that gio-2.0.pc names (4.20.x asks pkg-config for it).
CONF_ENV=( WAYLAND_SCANNER="$WS/hosttools/bin/wayland-scanner" GLIB_COMPILE_RESOURCES=/usr/bin/glib-compile-resources )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-vala --disable-x11
                  --enable-wayland --enable-gtk-layer-shell )
