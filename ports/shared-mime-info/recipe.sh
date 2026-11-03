# shared-mime-info: the freedesktop.org MIME type database. The database is compiled on the target with update-mime-database (it is not run at build time).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.4
TARBALL=https://gitlab.freedesktop.org/xdg/shared-mime-info/-/archive/2.4/shared-mime-info-2.4.tar.gz
SHA256=531291d0387eb94e16e775d7e73788d06d2b2fdd8cd2ac6b6b15287593b6a2de
DEPS="glib libxml2"
# apk: rebuild the database whenever a package changes the MIME definitions
# (ports/shared-mime-info/apk.trigger; manual installs: docs/packages.md)
APK_TRIGGERS="/usr/local/share/mime/packages"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dupdate-mimedb=false -Dbuild-tests=false -Dbuild-translations=false )
