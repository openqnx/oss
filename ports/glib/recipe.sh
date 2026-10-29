VERSION=2.82.4; GIT=https://gitlab.gnome.org/GNOME/glib.git; REF=2.82.4
DEPS="libffi pcre2 gettext zlib"
BUILDSYS=meson
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
