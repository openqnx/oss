VERSION=1.10.8; GIT=https://github.com/ebassi/graphene.git; REF=1.10.8
DEPS="glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dinstalled_tests=false -Dgtk_doc=false
                   -Dintrospection=disabled -Dgobject_types=true )
