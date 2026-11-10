# PyGObject: Python bindings for GObject-based libraries through GObject
# introspection (gi.repository.GLib, GObject, Gio, ...), on glib's
# libgirepository-2.0. Its C extensions (gi._gi, gi._gi_cairo) are built with
# meson-python, cross compiled against the ported glib, libffi, cairo and
# pycairo (scripts/lib/pywheel.sh). The introspection data it loads at run
# time -- GLib-2.0.typelib and friends -- comes from glib-typelibs.
# pycairo is found through a py3cairo.pc written here (its header is in the
# staged pycairo package, cairo/include): PyGObject's first way, asking the
# build machine's Python for the cairo module, finds nothing in the venv,
# and the build requirement pycairo is not installed there (-x: the build
# machine's pycairo would be a build of the build machine's cairo).
VERSION=3.58.1
TARBALL=https://files.pythonhosted.org/packages/65/1e/d590f82e14db75945b03f6b63e0f19662e0340babd8f574fdb1bb5a015e2/pygobject-3.58.1.tar.gz
SHA256=4c80598ade17fbaa7798e01a25d0bf29ce109740786026a074c5c62bb3e79d23
DEPS="python3 glib libffi cairo python3-cairo glib-typelibs"
RUNTIME_DEPS="python3 python3-cairo glib-typelibs"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() {
  qnx_py_venv "meson-python>=0.16" "meson>=1.2" && qnx_py_crossfile || return 1
  mkdir -p "$B/pc" && cat > "$B/pc/py3cairo.pc" <<PC || return 1
prefix=$PREFIX
includedir=\${prefix}/lib/python$QNX_PYV/site-packages/cairo/include
Name: Pycairo
Description: Python bindings for cairo (staged package)
Version: $(sed -n 's/^VERSION=//p' "$WS/ports/python3-cairo/recipe.sh")
Cflags: -I\${includedir}
PC
  sed -i "s#^pkg_config_libdir = \['\([^']*\)'\]#pkg_config_libdir = ['\1', '$STAGE$PREFIX/share/pkgconfig', '$B/pc']#" "$B/qnx-cross.ini"
}
do_build() {
  qnx_py_wheel -x -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" \
    -Csetup-args="--native-file=$B/qnx-native.ini" -Csetup-args="-Dtests=false" \
    -Csetup-args="-Dpycairo=enabled" -Csetup-args="--wrap-mode=nodownload"
}
do_install() { qnx_py_install; }
