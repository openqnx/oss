VERSION=1.14.53; GIT=https://gitlab.gnome.org/GNOME/libgsf.git; REF=LIBGSF_1_14_53
DEPS="glib libxml2 zlib bzip2"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-gdk-pixbuf
                  --disable-introspection --disable-gtk-doc )
# gtk-doc is not installed here. scripts/build-host-tools.sh provides both halves
# of what autoreconf needs: a GTK_DOC_CHECK m4 stub and a gtkdocize stub that
# writes the disabled-documentation gtk-doc.make.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
