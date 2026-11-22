# xfce4-settings: the settings manager and dialogs.
# The QNX Everywhere 8.0.4r1 image has the 4.21 development snapshot; the
# 4.20 release is used here to match the Xfce 4.20 libraries in this tree.
VERSION=4.20.5
TARBALL=https://archive.xfce.org/src/xfce/xfce4-settings/4.20/xfce4-settings-4.20.5.tar.bz2
SHA256=a5fbe0e511cce29d603320ade575ad4001bd570e60f37760233237ba478affe8
# Also linked at run time (found by scripts/gen-dependency-index): gtk-layer-shell
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell"
BUILDSYS=autotools
# glib-compile-resources runs at build time: the build host's, not the staged QNX
# one that gio-2.0.pc names (4.20.x asks pkg-config for it).
CONF_ENV=( WAYLAND_SCANNER="$WS/hosttools/bin/wayland-scanner" GLIB_COMPILE_RESOURCES=/usr/bin/glib-compile-resources )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc --disable-x11 --enable-wayland --disable-xrandr --disable-xcursor --disable-libnotify --disable-colord --disable-upower-glib )
