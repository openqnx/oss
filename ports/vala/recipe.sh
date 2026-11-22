# Vala: the compiler (valac-0.56, with valac and vapigen) that translates
# Vala -- a C#-like language over GLib/GObject -- to C, plus libvala-0.56 and
# the VAPI bindings (share/vala-0.56/vapi). The release archive carries the
# compiler's own C translation, so it builds with a C compiler and GLib, no
# Vala needed. On a QNX target without a C compiler, valac -C (and
# --header, --vapi) produce the C sources to build elsewhere. Version as in
# QNX's own package list.
VERSION=0.56.19
TARBALL=https://download.gnome.org/sources/vala/0.56/vala-0.56.19.tar.xz
SHA256=5ad7cbbfcc0de61b403d6797c9ef60455bfbebd8e162aec33b5b0b097adfb9d5
BUILDSYS=autotools
DEPS="glib gettext"
# valadoc needs graphviz (libgvc); not built.
CONF_ARGS_EXTRA=( --disable-valadoc )
