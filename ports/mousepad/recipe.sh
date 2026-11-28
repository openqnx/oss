# mousepad: the Xfce text editor (GTK 3, GtkSourceView 4) -- the plain text
# editor of Raspberry Pi OS's desktop: tabs, syntax highlighting, search and
# replace, encodings, recent files. Release series 0.6 (Xfce 4.20).
VERSION=0.6.5
TARBALL=https://archive.xfce.org/src/apps/mousepad/0.6/mousepad-0.6.5.tar.xz
SHA256=21762bc8c3c4f120a4a509ce39f4a5a58dbc10e3f0da66cdc6d9a8c735fff2ac
# libxfce4ui: the keyboard shortcut editor plugin.
DEPS="glib gtk3 gtksourceview4 libxfce4ui"
RUNTIME_DEPS="gtksourceview4 libxfce4ui gsettings-desktop-schemas"
BUILDSYS=meson
# - GtkSourceView 4 (not 3, which is not ported);
# - no polkit (editing files of other users as root through pkexec: no polkit
#   on QNX) and no gspell (the spell checker library is not ported);
# - settings in GSettings (dconf in the desktop), not the keyfile backend.
MESON_ARGS_EXTRA=( -Dgtksourceview4=enabled -Dpolkit=disabled -Dgspell-plugin=disabled
                   -Dshortcuts-plugin=enabled -Dtest-plugin=disabled )
