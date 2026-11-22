VERSION=4.20.2
# Release tarball, as with the rest of Xfce -- see README hazard 74.
TARBALL=https://archive.xfce.org/src/xfce/tumbler/4.20/tumbler-4.20.2.tar.bz2
SHA256=aa77fd90a88b83467cd8d2b39d80e4518ba2903d2e4ca50d32d1b96996685015
# Also linked at run time (found by scripts/gen-dependency-index): curl gst-plugins-base gstreamer
DEPS="glib dbus gdk-pixbuf libgsf ffmpegthumbnailer libpng freetype libxfce4util curl gst-plugins-base gstreamer"
BUILDSYS=autotools
# The Xfce thumbnailing service.  Its plugins are what make it worth porting:
# the ffmpeg thumbnailer and the GSF (OLE2/ZIP document) one both have their
# backing libraries staged already.  No poppler (PDF) or libgepub here -- not
# ported -- and no systemd login handling.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-libgepub --disable-poppler
                  --enable-ffmpeg --enable-gsf --enable-pixbuf-thumbnailer
                  --enable-font-thumbnailer )
