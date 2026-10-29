# glib-networking: TLS (GnuTLS) and proxy support for GIO.
VERSION=2.80.1; GIT=https://gitlab.gnome.org/GNOME/glib-networking.git; REF=2.80.1
DEPS="glib gnutls libproxy"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dgnutls=enabled -Dopenssl=disabled -Dlibproxy=enabled -Dgnome_proxy=disabled -Dinstalled_tests=false )
