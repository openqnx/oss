VERSION=3.6.1; GIT=https://gitlab.gnome.org/GNOME/libsoup.git; REF=3.6.1
# Also linked at run time (found by scripts/gen-dependency-index): brotli
DEPS="glib libxml2 sqlite nghttp2 libpsl brotli"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dintrospection=disabled -Dvapi=disabled
                   -Ddocs=disabled -Dsysprof=disabled -Dgssapi=disabled
                   -Dkrb5_config= -Dntlm=disabled -Dbrotli=enabled -Dtls_check=false )
