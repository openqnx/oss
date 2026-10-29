VERSION=3.44.0; GIT=https://gitlab.gnome.org/GNOME/libdazzle.git; REF=3.44.0
DEPS="glib gtk3"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Denable_tests=false -Denable_tools=false
                   -Dwith_introspection=false -Dwith_vapi=false -Denable_gtk_doc=false )
