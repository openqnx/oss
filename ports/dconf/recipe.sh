# dconf: the GSettings storage of the GNOME/GTK desktop -- a binary key/value
# database per user (~/.config/dconf/user), the D-Bus service that writes it
# (ca.desrt.dconf, started by the session bus on first use), the GIO module that
# makes it the GSettings backend (lib/gio/modules/libdconfsettings.so), the
# client library and the dconf command (read, write, list, dump, load, watch,
# reset, compile, update). With it GTK programs keep their settings between
# runs; without it GSettings falls back to the memory backend.
VERSION=51.0
TARBALL=https://download.gnome.org/sources/dconf/51/dconf-51.0.tar.xz
SHA256=e65c1b7867f836faad9f9ee04acf5c5a6cae2bbc784833fe093207e0cb590248
# dbus: the build reads session_bus_services_dir from dbus-1.pc, and at run time
# readers work on the mapped database alone but every write goes through
# dconf-service on the session bus.
DEPS="glib dbus"
RUNTIME_DEPS="dbus"
BUILDSYS=meson
# - no man pages: they are made with xsltproc from DocBook stylesheets fetched
#   by URL; no gtk-doc API reference;
# - no Vala binding (vapigen is asked for only to find the vapi directory) and
#   no bash completion file (it would add bash-completion as a build dependency);
# - systemduserunitdir: QNX has no systemd. The unit is installed whatever the
#   option says, so it is given a path of its own and removed after the install.
MESON_ARGS_EXTRA=( -Dman=false -Dgtk_doc=false -Dvapi=false -Dbash_completion=false
                   -Dsystemduserunitdir=$PREFIX/lib/systemd/user )
do_install() { DESTDIR="$STAGE" ninja -C "$B" install && rm -rf "$STAGE$PREFIX/lib/systemd"; }
