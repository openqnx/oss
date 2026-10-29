VERSION=1.10.6; GIT=https://gitlab.gnome.org/GNOME/json-glib.git; REF=1.10.6
DEPS="glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dintrospection=disabled -Dgtk_doc=disabled -Dtests=false
                   -Dinstalled_tests=false )
