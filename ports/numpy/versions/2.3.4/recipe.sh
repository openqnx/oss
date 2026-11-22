VERSION=2.3.4; GIT=https://github.com/numpy/numpy.git; REF=v2.3.4
DEPS="python3 openblas"
BUILDSYS=custom
# A Python extension package, cross compiled the way qnx-ports does it:
#  - the build runs in a venv of the *host* Python of the same version as the
#    target's (hosttools/bin/python3.14), with numpy's build requirements;
#  - _PYTHON_SYSCONFIGDATA_NAME points it at the target's sysconfig data
#    (staged by the python3 recipe), so extension names and flags are QNX's;
#  - meson-python gets a cross file for the QNX compiler and the long double
#    format of each architecture (quad on aarch64, x87 80-bit on x86_64).
# The wheel is then unpacked into the target's site-packages in staging.
# The steps are shared by every Python package: scripts/lib/pywheel.sh.
source "$WS/scripts/lib/pywheel.sh"
do_configure() {
  qnx_py_venv "meson-python>=0.18" "Cython>=3.0.6" "meson>=1.5" "patchelf>=0.11.0" || return 1
  qnx_py_crossfile "longdouble_format = '$([ "$QNX_ARCH" = aarch64le ] && echo IEEE_QUAD_LE || echo INTEL_EXTENDED_16_BYTES_LE)'"
}
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" -Csetup-args="--native-file=$B/qnx-native.ini" \
    -Csetup-args="-Dallow-noblas=false" -Csetup-args="-Dblas=openblas" -Csetup-args="-Dlapack=openblas"
}
do_install() { qnx_py_install; }
