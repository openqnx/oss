# contourpy: contour line and filled-contour calculation for Python (C++
# with pybind11), matplotlib's contouring engine. Cross compiled like numpy
# (scripts/lib/pywheel.sh): meson-python with a cross file for the QNX
# compilers.
VERSION=1.4.0
TARBALL=https://files.pythonhosted.org/packages/83/5a/a55177dd22553a277388e8a1b3220e92de91bacb28356cdc73caa240121d/contourpy-1.4.0.tar.gz
SHA256=20156f5a1ac4f8ce02656e39a61e82164a3d359796dc8026f75b062783d500e1
DEPS="numpy"
RUNTIME_DEPS="numpy"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
# pybind11 (headers only) comes from the build venv; Meson finds it through
# its config tool, which a cross build looks up in the cross file.
do_configure() {
  qnx_py_venv "meson-python>=0.18" "meson>=1.2" "pybind11>=2.13.2,!=2.13.3" "patchelf>=0.11.0" || return 1
  qnx_py_crossfile && echo "pybind11-config = '$B/venv/bin/pybind11-config'" >> "$B/qnx-cross.ini"
}
# werror off: the project builds with -Werror, and GCC 12 warns differently
# from the compilers upstream tests with.
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" \
    -Csetup-args="--native-file=$B/qnx-native.ini" -Csetup-args="-Dwerror=false"
}
do_install() { qnx_py_install; }
