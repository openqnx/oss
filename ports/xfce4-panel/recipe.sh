VERSION=4.20.0
# Release tarball, as for every Xfce package here -- see README hazard 74.
TARBALL=https://archive.xfce.org/src/xfce/xfce4-panel/4.20/xfce4-panel-4.20.0.tar.bz2
SHA256=ff33cd5f5d16c2193fe305f4878d82cd8d2feea92f2594bcd27b2b5c392d43b8
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell wayland wayland-protocols"
BUILDSYS=autotools
# Wayland only, matching libxfce4windowing, which was built --disable-x11: the
# panel's X11 half needs libwnck, and libwnck's window tracking is X11 through
# and through.  gtk-layer-shell is what lets the panel anchor itself to a screen
# edge under a wlroots-style compositor.
CONF_ENV=( WAYLAND_SCANNER="$WS/hosttools/bin/wayland-scanner" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-vala --disable-x11
                  --enable-wayland --enable-gtk-layer-shell )
