VERSION=2.90.1; GIT=https://gitlab.gnome.org/GNOME/glib.git; REF=2.90.1
DEPS="libffi pcre2 gettext zlib"
# apk: compile the GSettings schemas whenever a package changes this directory
# (ports/glib/apk.trigger; manual installs: docs/packages.md)
APK_TRIGGERS="/usr/local/share/glib-2.0/schemas"
BUILDSYS=meson
# glib 2.90 needs Meson >= 1.4, newer than the build image's default
HOSTTOOLS_NEW=1
MESON_ARGS_EXTRA=(
  -Dtests=false
  -Dglib_debug=disabled
  -Dman-pages=disabled
  -Ddocumentation=false
  -Dintrospection=disabled          # needs a working gobject-introspection scanner on target
  -Dlibmount=disabled               # Linux util-linux only
  -Dselinux=disabled                # Linux only
  -Dxattr=false                     # QNX has no <sys/xattr.h>
  -Dnls=enabled
  -Dtests=false
  -Dsysprof=disabled
  -Ddtrace=false
  -Dsystemtap=false
)
