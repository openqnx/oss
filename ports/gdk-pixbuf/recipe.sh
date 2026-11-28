VERSION=2.44.8; GIT=https://gitlab.gnome.org/GNOME/gdk-pixbuf.git; REF=2.44.8
DEPS="glib libpng libjpeg-turbo"
BUILDSYS=meson
# apk.trigger: loaders.cache rewritten whenever a package adds or removes a
# loader module (librsvg's SVG loader); the built-in loaders need none.
APK_TRIGGERS="/usr/local/lib/gdk-pixbuf-2.0/2.10.0/loaders"
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
# The package owns the loaders/ directory (through a small README in it; qnxpkg
# packs files, not empty directories): then installing gdk-pixbuf changes the
# trigger directory, so its trigger also runs when librsvg and gdk-pixbuf were
# removed together (no trigger ran then) and a stale loaders.cache naming the
# SVG module was left behind -- the reinstall now deletes it (GTK programs
# aborted on it: "Unable to load image-loading module"); and apk no longer
# takes the directory away with the last loader package.
do_install() {
  DESTDIR="$STAGE" ninja -C "$B" install
  mkdir -p "$STAGE$PREFIX/lib/gdk-pixbuf-2.0/2.10.0/loaders"
  printf '%s\n' "gdk-pixbuf loader modules of other packages (librsvg's SVG loader, ...)." \
    "The gdk-pixbuf apk trigger rewrites ../loaders.cache whenever they change." \
    > "$STAGE$PREFIX/lib/gdk-pixbuf-2.0/2.10.0/loaders/README"
}
