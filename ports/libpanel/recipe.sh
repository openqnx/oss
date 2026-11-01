# libpanel: dock/panel layout library for GTK 4 and libadwaita (resizable
# docks and paned layouts, frames with tabs, widgets that move between
# them, save/restore of the layout, the omni bar) used by GNOME Builder.
VERSION=1.10.4
TARBALL=https://download.gnome.org/sources/libpanel/1.10/libpanel-1.10.4.tar.xz
SHA256=593888a7691f0af8aaa6e193c9e14afa86a810c0c2f27515c6d813f18733b1cd
DEPS="glib gtk4 libadwaita"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dintrospection=disabled -Dvapi=false -Ddocs=disabled -Dinstall-examples=false )
