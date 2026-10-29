# gThumb image viewer and organiser.
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=3.12.8.2
TARBALL=https://download.gnome.org/sources/gthumb/3.12/gthumb-3.12.8.2.tar.xz
SHA256=abc57b1103165dd691535796f7dbdba7686217c7d069e974ed0f3d800fe88796
DEPS="glib gtk3 gdk-pixbuf libjpeg-turbo libpng libtiff libwebp lcms2 exiv2 libsecret json-glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dlibtiff=true -Dlibwebp=true -Dlcms2=true -Dlibraw=false -Dlibheif=false -Dlibjxl=false -Dcolord=false -Dgstreamer=false -Dlibchamplain=false -Dlibbrasero=false -Dclutter=false -Dlibrsvg=false )
