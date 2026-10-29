# Geany, the lightweight IDE (GTK3).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.1
TARBALL=https://download.geany.org/geany-2.1.tar.bz2
SHA256=6b96a8844463300c10b9692a0a5edad8236eec9e84342f575f83d4fc89331228
DEPS="glib gtk3"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-html-docs --disable-pdf-docs --disable-api-docs --disable-gtkdoc-header --disable-vte )
