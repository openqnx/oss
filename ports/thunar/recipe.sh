VERSION=4.20.10
TARBALL=https://archive.xfce.org/src/xfce/thunar/4.20/thunar-4.20.10.tar.bz2
SHA256=a5a32b51028dc821155e44cdec025fe70398bae193c619ae6ba8b1babf6f49f1
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf exo pango gdk-pixbuf libnotify libexif pcre2 xfce4-panel"
BUILDSYS=autotools
# The Xfce file manager, and the thunarx extension library that its plugins
# (and tumbler's consumers) link against.  gudev is Linux's udev, which QNX
# does not have, so volume management is off; polkit likewise.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-gudev --disable-polkit
                  --enable-notifications --enable-exif --enable-pcre2 )
# The release tarball ships the gdbus-codegen output, made with a glib newer
# than the target's: it calls g_variant_builder_init_static (glib 2.84), and
# the XML for two of the four files is not in the tarball to make them again.
# The call takes the same arguments as g_variant_builder_init (both set up a
# builder on the stack; _static only skips copying the type): use that one.
do_patch() {
  cd "$SRCROOT/thunar" || return 1
  sed -i 's/g_variant_builder_init_static (/g_variant_builder_init (/g' \
    thunar-dbus-freedesktop-interfaces.c thunar-dbus-service-infos.c \
    thunar-thumbnailer-proxy.c thunar-thumbnail-cache-proxy.c &&
  ! grep -q g_variant_builder_init_static ./*.c
}
