VERSION=1.8.3; GIT=https://gitlab.gnome.org/GNOME/libhandy.git; REF=1.8.3
DEPS="glib gtk3"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dexamples=false -Dtests=false -Dintrospection=disabled
                   -Dvapi=false -Dgtk_doc=false -Dglade_catalog=disabled )
