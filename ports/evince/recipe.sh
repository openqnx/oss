# Evince: the GNOME document viewer (GTK 3) -- PDF through poppler, TIFF, and
# comic book archives (cbz/cbr/cb7/cbt through libarchive); search, outline and
# thumbnails side pane, annotations, forms, presentation mode, printing through
# the GTK print dialog. Also the evince-thumbnailer the file manager uses for
# document thumbnails, the print previewer, and libevdocument/libevview for
# programs that embed the viewer. 48 is the last GTK 3 series.
VERSION=48.5
TARBALL=https://download.gnome.org/sources/evince/48/evince-48.5.tar.xz
SHA256=0c7aba9485d0b6fb9f3e0ee75be916544744dc2ed9d58a2f9936a2e0434ac4a6
# libsecret: passwords of encrypted documents kept in the keyring; dbus: the
# build reads session_bus_services_dir from dbus-1.pc (evinced, the daemon
# that keeps one window per document); gsettings-desktop-schemas: the
# lockdown and interface settings, looked up when present.
DEPS="glib gtk3 gdk-pixbuf cairo libhandy libxml2 poppler libtiff libarchive libsecret dbus gsettings-desktop-schemas zlib"
# The org.gnome.desktop.* schemas g_settings_new() aborts without; the MIME
# database (a document is opened by its content type, and without
# shared-mime-info every file is application/octet-stream); the symbolic icons
# of the toolbar and side pane.
RUNTIME_DEPS="gsettings-desktop-schemas shared-mime-info adwaita-icon-theme dbus"
BUILDSYS=meson
# - formats: PDF, TIFF and comics. No DjVu (djvulibre), XPS (libgxps), DVI
#   (kpathsea) or PostScript (libspectre): those libraries are not ported;
# - thumbnail_cache: gnome-desktop (the recent-documents thumbnails) is not
#   ported; multimedia: video inside PDFs through GStreamer, which the viewer
#   embeds with an X11 window handle -- off on a Wayland desktop; gspell is not
#   ported (no spell checking in annotations);
# - internal_synctex: the bundled SyncTeX parser (no TeX Live here);
# - no Nautilus extension, GObject introspection, API reference, or user
#   manual (it is shown by yelp, which is not ported);
# - systemduserunitdir=no: QNX has no systemd; evinced is started by the
#   session bus (share/dbus-1/services).
MESON_ARGS_EXTRA=( -Dplatform=gnome -Dviewer=true -Dpreviewer=true -Dthumbnailer=true -Dnautilus=false
                   -Dpdf=enabled -Dtiff=enabled -Dcomics=enabled
                   -Ddjvu=disabled -Dxps=disabled -Ddvi=disabled -Dps=disabled
                   -Ddbus=true -Dkeyring=enabled -Dgtk_unix_print=enabled
                   -Dthumbnail_cache=disabled -Dmultimedia=disabled -Dgspell=disabled
                   -Dinternal_synctex=true -Dintrospection=false -Dgtk_doc=false -Duser_doc=false
                   -Dsystemduserunitdir=no )
