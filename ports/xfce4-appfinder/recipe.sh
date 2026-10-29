# xfce4-appfinder: application finder/launcher.
# The QNX Everywhere 8.0.4r1 image has the 4.21 development snapshot; the
# 4.20 release is used here to match the Xfce 4.20 libraries in this tree.
VERSION=4.20.0
TARBALL=https://archive.xfce.org/src/xfce/xfce4-appfinder/4.20/xfce4-appfinder-4.20.0.tar.bz2
SHA256=82ca82f77dc83e285db45438c2fe31df445148aa986ffebf2faabee4af9e7304
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf garcon"
BUILDSYS=autotools
CONF_ENV=( WAYLAND_SCANNER="$WS/hosttools/bin/wayland-scanner" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc --disable-x11 --enable-wayland  )
