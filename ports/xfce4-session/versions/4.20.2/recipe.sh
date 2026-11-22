# xfce4-session: session manager.
# The QNX Everywhere 8.0.4r1 image has the 4.21 development snapshot; the
# 4.20 release is used here to match the Xfce 4.20 libraries in this tree.
VERSION=4.20.2
TARBALL=https://archive.xfce.org/src/xfce/xfce4-session/4.20/xfce4-session-4.20.2.tar.bz2
SHA256=a0b80b7136515bc3c0c54fa859ad420365e29b715b6da0b58a2d2781bfbe73c3
# Also linked at run time (found by scripts/gen-dependency-index): gtk-layer-shell
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf libxfce4windowing gtk-layer-shell"
BUILDSYS=autotools
# glib-compile-resources runs at build time: the build host's, not the staged QNX one.
CONF_ENV=( WAYLAND_SCANNER="$WS/hosttools/bin/wayland-scanner" GLIB_COMPILE_RESOURCES=/usr/bin/glib-compile-resources )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc --disable-x11 --enable-wayland --disable-polkit )
