VERSION=4.20.0
TARBALL=https://archive.xfce.org/src/xfce/thunar/4.20/thunar-4.20.0.tar.bz2
SHA256=27731a76f3aecf3752b1ca35afad89e264c52244f70083d933507dd4a17548b0
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf exo pango gdk-pixbuf libnotify libexif pcre2 xfce4-panel"
BUILDSYS=autotools
# The Xfce file manager, and the thunarx extension library that its plugins
# (and tumbler's consumers) link against.  gudev is Linux's udev, which QNX
# does not have, so volume management is off; polkit likewise.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-gudev --disable-polkit
                  --enable-notifications --enable-exif --enable-pcre2 )
