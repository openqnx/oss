# pycairo: Python bindings for the cairo 2D graphics library (surfaces --
# image, PDF, PS, SVG, recording -- paths, patterns, text). A C extension
# (cairo._cairo) built with meson-python, cross compiled against the ported
# cairo like numpy (scripts/lib/pywheel.sh). -Dwheel=true keeps the C API
# header (py3cairo.h, used by PyGObject's cairo integration) inside the
# package, in cairo/include. No X11 surfaces (no Xlib on QNX).
VERSION=1.29.2
TARBALL=https://files.pythonhosted.org/packages/bb/5e/19ab2572981a996ce96b853a189d2dcced40506b67d9555a7568457abcf2/pycairo-1.29.2.tar.gz
SHA256=3e69fff74fe64f5ba2dfa31f67c6bdf26413342574047437d2ac520d35e9a489
DEPS="python3 cairo"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
# cairo's .pc requires zlib's and xproto's, which are in share/pkgconfig.
do_configure() {
  qnx_py_venv "meson-python>=0.16" "meson>=1.2" "patchelf>=0.11.0" && qnx_py_crossfile || return 1
  sed -i "s#^pkg_config_libdir = \['\([^']*\)'\]#pkg_config_libdir = ['\1', '$STAGE$PREFIX/share/pkgconfig']#" "$B/qnx-cross.ini"
}
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" \
    -Csetup-args="--native-file=$B/qnx-native.ini" -Csetup-args="-Dtests=false" \
    -Csetup-args="-Dwheel=true" -Csetup-args="-Dno-x11=true" -Csetup-args="--wrap-mode=nodownload"
}
do_install() { qnx_py_install; }
