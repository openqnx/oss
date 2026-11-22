VERSION=0.8.9; GIT=https://gitlab.gnome.org/GNOME/libnotify.git; REF=0.8.9
DEPS="glib gdk-pixbuf"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dintrospection=disabled -Dman=false -Dgtk_doc=false
                   -Ddocbook_docs=disabled )
