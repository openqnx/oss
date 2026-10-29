VERSION=0.78.2
TARBALL=https://download.gnome.org/sources/vte/0.78/vte-0.78.2.tar.xz
SHA256=35d7bcde07356846b4a12881c8e016705b70a9004a9082285eee5834ccc49890
# Also linked at run time (found by scripts/gen-dependency-index): lz4
DEPS="gtk3 gtk4 pcre2 fribidi icu glib gnutls lz4"
BUILDSYS=meson
# The terminal widget behind xfce4-terminal and gnome-terminal.  Both toolkit
# bindings are built because both GTK3 and GTK4 are staged.
# -D_systemd=false: no systemd scope/cgroup handling on QNX.
# -Dvapi / -Dgir false: Vala and introspection both need to run target tools.
MESON_ARGS_EXTRA=( -Dgtk3=true -Dgtk4=true -Dgir=false -Dvapi=false
                   -Ddocs=false -D_systemd=false -Dglade=false
                   -Dicu=true -Dgnutls=true )
