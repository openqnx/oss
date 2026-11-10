# glib-typelibs: the GObject introspection data of the ported GLib --
# GLib-2.0, GObject-2.0, Gio-2.0, GModule-2.0, GLibUnix-2.0, GioUnix-2.0 and
# GIRepository-3.0 (.typelib for lib/girepository-1.0, .gir for share/gir-1.0)
# -- which PyGObject (python3-gobject3) loads to reach GLib from Python.
#
# The glib port cannot make them (-Dintrospection=disabled): g-ir-scanner
# runs a "dumper" program linked against the library it describes, and a QNX
# program does not run on the build machine. So the same GLib release is
# built a second time natively, on the build machine, only to describe it:
#   1. GLib (no introspection), 2. gobject-introspection's g-ir-scanner
#   against it, 3. GLib again with -Dintrospection=enabled,
# all in build/host/glib-typelibs, once for both architectures.
# The API description is the same on QNX (same sources, same version); both
# QNX targets are LP64 little endian like the build machine, so the sizes
# and offsets g-ir-compiler computes match. What does differ are the values
# glibconfig.h gives the GLIB_SYSDEF_* constants (AF_INET6, MSG_*): those are
# taken from the target's staged glibconfig.h before the .typelibs are
# compiled (fix-constants.py).
VERSION=2.82.4
GIT=https://gitlab.gnome.org/GNOME/glib.git; REF=2.82.4
GI_GIT=https://gitlab.gnome.org/GNOME/gobject-introspection.git; GI_REF=1.86.0
DEPS="glib"
RUNTIME_DEPS=""
BUILDSYS=custom
do_fetch() {
  git clone -q --depth 1 --branch "$REF" "$GIT" "$SRCROOT/glib" &&
  git clone -q --depth 1 --branch "$GI_REF" "$GI_GIT" "$SRCROOT/gobject-introspection" || { rm -rf "$SRCROOT"; return 1; }
}
# The native builds: the build machine's compiler, pkg-config and libraries
# (libffi, pcre2, zlib, libmount), nothing of the cross environment.
_host() {
  env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u LDFLAGS -u CPPFLAGS -u PKG_CONFIG -u PKG_CONFIG_LIBDIR \
      -u PKG_CONFIG_SYSROOT_DIR -u QNX_TARGET -u SYSROOT \
      PATH="$H/root/bin:$H/venv/bin:/usr/bin:/bin" PKG_CONFIG_PATH="$H/root/lib/pkgconfig" \
      LD_LIBRARY_PATH="$H/root/lib" "$@"
}
_native() {
  H="$BUILD_DIR/host/glib-typelibs"
  [ -f "$H/done-$VERSION-$GI_REF" ] && return 0
  rm -rf "$H" && mkdir -p "$H" || return 1
  /usr/bin/python3 -m venv "$H/venv" && "$H/venv/bin/pip" install -q "meson>=1.6,<1.9" setuptools packaging || return 1
  local common=( --prefix="$H/root" --libdir=lib --buildtype=release --wrap-mode=nodownload )
  local gopts=( -Dtests=false -Dman-pages=disabled -Ddocumentation=false -Dnls=disabled -Dselinux=disabled
                -Dsysprof=disabled -Dlibmount=disabled -Dxattr=false -Ddtrace=disabled -Dsystemtap=disabled )
  _host meson setup "$H/glib1" "$SRCROOT/glib" "${common[@]}" "${gopts[@]}" -Dintrospection=disabled &&
  _host ninja -C "$H/glib1" install || return 1
  _host meson setup "$H/gi" "$SRCROOT/gobject-introspection" "${common[@]}" -Dbuild_introspection_data=false \
      -Dgtk_doc=false -Ddoctool=disabled -Dcairo=disabled -Dtests=false -Dpython="$H/venv/bin/python" &&
  _host ninja -C "$H/gi" install || return 1
  _host meson setup "$H/glib2" "$SRCROOT/glib" "${common[@]}" "${gopts[@]}" -Dintrospection=enabled &&
  _host ninja -C "$H/glib2" || return 1
  touch "$H/done-$VERSION-$GI_REF"
}
do_build() {
  _native || return 1
  rm -rf "$B/gir" "$B/typelib" && mkdir -p "$B/gir" "$B/typelib" || return 1
  find "$H/glib2" -name '*.gir' -exec cp {} "$B/gir/" \; || return 1
  ls "$B/gir"
  /usr/bin/python3 "$WS/ports/glib-typelibs/fix-constants.py" "$STAGE$PREFIX/lib/glib-2.0/include/glibconfig.h" "$SRCROOT/glib" "$B/gir"/*.gir || return 1
  local g
  for g in "$B/gir"/*.gir; do
    _host gi-compile-repository --includedir="$B/gir" -o "$B/typelib/$(basename "${g%.gir}").typelib" "$g" || return 1
  done
}
do_install() {
  mkdir -p "$STAGE$PREFIX/lib/girepository-1.0" "$STAGE$PREFIX/share/gir-1.0"
  cp "$B"/typelib/*.typelib "$STAGE$PREFIX/lib/girepository-1.0/"
  cp "$B"/gir/*.gir "$STAGE$PREFIX/share/gir-1.0/"
}
