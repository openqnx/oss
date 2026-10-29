# GObject Introspection: libgirepository and the g-ir tools for the target.
# The introspection data (.gir/.typelib) of GLib itself cannot be generated
# in a cross build without running target binaries, so it is not built here
# (build_introspection_data=false). Version as in the QNX Everywhere image.
VERSION=1.86.0; GIT=https://gitlab.gnome.org/GNOME/gobject-introspection.git; REF=1.86.0
DEPS="glib libffi python3"
BUILDSYS=meson
QNX_PYV=3.14
# The giscanner extension module is built against the target's Python (the
# python3 port); the build host's Python of the same version describes it.
MESON_ARGS_EXTRA=( -Dbuild_introspection_data=false -Dgtk_doc=false -Ddoctool=disabled -Dcairo=disabled -Dpython=$BUILD_DIR/host/gi-venv/bin/python -Dc_args=-I$STAGE$PREFIX/include/python$QNX_PYV )
# gobject-introspection 1.86 needs Meson >= 1.4; it gets its own from a venv of
# the build host's Python, so the host's Meson is left as it is.
BUILDSYS=custom
do_configure() {
  local v="$BUILD_DIR/host/meson-venv"
  [ -x "$v/bin/meson" ] || { python3 -m venv "$v" && "$v/bin/pip" install -q "meson>=1.6,<1.9"; } || return 1
  local g="$BUILD_DIR/host/gi-venv"
  [ -x "$g/bin/python" ] || { "$WS/hosttools/bin/python$QNX_PYV" -m venv "$g" && "$g/bin/pip" install -q setuptools; } || return 1
  "$v/bin/meson" setup "$B" "$S" --cross-file "$QNX_MESON_DIR/meson-$QNX_ARCH.ini" --native-file "$QNX_MESON_DIR/meson-native.ini" \
    --prefix="$PREFIX" --buildtype=release "${MESON_ARGS_EXTRA[@]}"
}
do_build() { "$BUILD_DIR/host/meson-venv/bin/meson" compile -C "$B"; }
do_install() { DESTDIR="$STAGE" "$BUILD_DIR/host/meson-venv/bin/meson" install -C "$B"; }
