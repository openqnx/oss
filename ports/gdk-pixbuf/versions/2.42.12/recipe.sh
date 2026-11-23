VERSION=2.42.12; GIT=https://gitlab.gnome.org/GNOME/gdk-pixbuf.git; REF=2.42.12
DEPS="glib libpng libjpeg-turbo"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dinstalled_tests=false -Dman=false
                   -Dgtk_doc=false -Dintrospection=disabled
                   -Dpng=enabled -Djpeg=enabled -Dtiff=disabled
                   -Dothers=enabled -Dbuiltin_loaders=all
                   -Drelocatable=false
                   # gio_sniffing pulls in shared-mime-info, a desktop data
                   # package; gdk-pixbuf's own image sniffing is what an
                   # embedded target wants anyway.
                   -Dgio_sniffing=false )
