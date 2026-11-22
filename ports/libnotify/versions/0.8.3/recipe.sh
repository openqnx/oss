VERSION=0.8.3; GIT=https://gitlab.gnome.org/GNOME/libnotify.git; REF=0.8.3
DEPS="glib gdk-pixbuf"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dintrospection=disabled -Dman=false -Dgtk_doc=false
                   -Ddocbook_docs=disabled )
