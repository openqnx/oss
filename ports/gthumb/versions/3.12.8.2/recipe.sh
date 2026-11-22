# gThumb image viewer and organiser.
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=3.12.8.2
TARBALL=https://download.gnome.org/sources/gthumb/3.12/gthumb-3.12.8.2.tar.xz
SHA256=abc57b1103165dd691535796f7dbdba7686217c7d069e974ed0f3d800fe88796
# The org.gnome.desktop.{background,interface,privacy,thumbnailers} schemas it
# opens with g_settings_new(), which aborts when a schema is not installed; and
# the MIME database: gthumb lists a file by its content type, and without
# shared-mime-info every file is application/octet-stream ("No file").
RUNTIME_DEPS="gsettings-desktop-schemas shared-mime-info"
DEPS="glib gtk3 gdk-pixbuf libjpeg-turbo libpng libtiff libwebp lcms2 exiv2 libsecret json-glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dlibtiff=true -Dlibwebp=true -Dlcms2=true -Dlibraw=false -Dlibheif=false -Dlibjxl=false -Dcolord=false -Dgstreamer=false -Dlibchamplain=false -Dlibbrasero=false -Dclutter=false -Dlibrsvg=false )
