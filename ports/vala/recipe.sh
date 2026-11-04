# Vala: the compiler (valac-0.56, with valac and vapigen) that translates
# Vala -- a C#-like language over GLib/GObject -- to C, plus libvala-0.56 and
# the VAPI bindings (share/vala-0.56/vapi). The release archive carries the
# compiler's own C translation, so it builds with a C compiler and GLib, no
# Vala needed. On a QNX target without a C compiler, valac -C (and
# --header, --vapi) produce the C sources to build elsewhere. Version as in
# QNX's own package list.
VERSION=0.56.18
TARBALL=https://download.gnome.org/sources/vala/0.56/vala-0.56.18.tar.xz
SHA256=f2affe7d40ab63db8e7b9ecc3f6bdc9c2fc7e3134c84ff2d795f482fe926a382
BUILDSYS=autotools
DEPS="glib gettext"
# valadoc needs graphviz (libgvc); not built.
CONF_ARGS_EXTRA=( --disable-valadoc )
