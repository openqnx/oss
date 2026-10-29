VERSION=0.21.4; GIT=https://gitlab.gnome.org/GNOME/libsecret.git; REF=0.21.4
DEPS="glib libgcrypt"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dintrospection=false -Dgtk_doc=false -Dmanpage=false
                   -Dvapi=false -Dcrypto=libgcrypt -Dtpm2=false -Dbash_completion=disabled )
