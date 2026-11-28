# gtksourceview4: GtkSourceView 4, the GTK 3 source-code editing widget
# (syntax highlighting, line numbers, search, undo, style schemes) -- the text
# area of Mousepad and other GTK 3 editors. The 4.x series is the GTK 3 one
# (5.x is for GTK 4).
VERSION=4.8.4
TARBALL=https://download.gnome.org/sources/gtksourceview/4.8/gtksourceview-4.8.4.tar.xz
SHA256=7ec9d18fb283d1f84a3a3eff3b7a72b09a10c9c006597b3fbabbb5958420a87d
DEPS="glib gtk3 libxml2 fribidi pango"
BUILDSYS=meson
# No GObject introspection data, Vala bindings, API docs, Glade catalog or
# installed tests.
MESON_ARGS_EXTRA=( -Dgir=false -Dvapi=false -Dgtk_doc=false -Dglade_catalog=false -Dinstall_tests=false )
