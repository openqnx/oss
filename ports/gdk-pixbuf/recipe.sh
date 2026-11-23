VERSION=2.44.8; GIT=https://gitlab.gnome.org/GNOME/gdk-pixbuf.git; REF=2.44.8
DEPS="glib libpng libjpeg-turbo"
BUILDSYS=meson
# 2.44 needs meson >= 1.5 (pinned default is 1.3.2)
HOSTTOOLS_NEW=1
MESON_ARGS_EXTRA=( -Dtests=false -Dinstalled_tests=false -Dman=false
                   -Ddocumentation=false -Dintrospection=disabled
                   -Dpng=enabled -Djpeg=enabled -Dtiff=disabled -Dgif=enabled
                   # glycin (Rust, sandboxed loaders) and the Android
                   # ImageDecoder loader do not apply to QNX.
                   -Dglycin=disabled -Dandroid=disabled
                   -Dothers=enabled -Dbuiltin_loaders=all
                   -Drelocatable=false
                   # gio_sniffing pulls in shared-mime-info, a desktop data
                   # package; gdk-pixbuf's own image sniffing is what an
                   # embedded target wants anyway.
                   -Dgio_sniffing=false )
