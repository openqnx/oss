# GSettings schemas shared by GNOME/GTK applications (data only).
VERSION=49.0; GIT=https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas.git; REF=49.0
DEPS="glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dintrospection=false )
