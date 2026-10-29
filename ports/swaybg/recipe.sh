# swaybg: the desktop background (wallpaper) on wlroots compositors such as labwc.
VERSION=1.2.2; GIT=https://github.com/swaywm/swaybg.git; REF=v1.2.2
DEPS="wayland wayland-protocols cairo gdk-pixbuf glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dgdk-pixbuf=enabled -Dman-pages=disabled )
