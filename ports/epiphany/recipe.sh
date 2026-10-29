# GNOME Web (Epiphany), the WebKitGTK browser.
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=43.1
TARBALL=https://download.gnome.org/sources/epiphany/43/epiphany-43.1.tar.xz
SHA256=e86ead27cb9982815150664de3bf20faf375f77b8065b02b31180c65b6bbebb4
# Also linked at run time (found by scripts/gen-dependency-index): libdazzle
DEPS="webkitgtk gtk3 libhandy libportal gcr libsecret json-glib sqlite nettle libarchive gsettings-desktop-schemas iso-codes libdazzle"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dunit_tests=disabled -Dtech_preview=false )
