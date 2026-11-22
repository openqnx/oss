VERSION=0.21.8.2; GIT=https://gitlab.gnome.org/GNOME/libsecret.git; REF=0.21.8.2
DEPS="glib libgcrypt"
BUILDSYS=meson
# 0.21.8 builds its own static secret-1 for the tests next to library():
# with --default-library=both (qnxbuild's) the two collide, so shared only.
MESON_ARGS_EXTRA=( --default-library=shared -Dintrospection=false -Dgtk_doc=false -Dmanpage=false
                   -Dvapi=false -Dcrypto=libgcrypt -Dtpm2=false -Dbash_completion=disabled )
