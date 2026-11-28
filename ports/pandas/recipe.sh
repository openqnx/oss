VERSION=2.3.3; GIT=https://github.com/pandas-dev/pandas.git; REF=v2.3.3
DEPS="numpy python3-dateutil python3-pytz python3-tzdata"
BUILDSYS=custom
# pandas 2.3.3: the first release line that supports Python 3.14 (the
# target's). Cross built like numpy (scripts/lib/pywheel.sh). pandas finds
# numpy's C headers by asking the build-host numpy for get_include().
#
# pandas 2.3.3 is built against numpy 2.3 (PANDAS_BUILD_NUMPY), not the
# target's newer numpy: built against numpy 2.5 headers, Timestamp.as_unit()
# and date_range() crash (SIGSEGV) on QNX, while the same pandas built against
# numpy 2.3.4 passes test_pandas with numpy 2.5.5 at run time -- numpy's C ABI
# is forward compatible, so building against the older numpy is the usual way.
# The venv's numpy 2.3.4 headers equal the QNX build's except that QNX has no
# <endian.h> (NPY_HAVE_ENDIAN_H): that line is switched off here, and
# npy_endian.h then takes the compiler's __BYTE_ORDER__ like on QNX.
# qnx-ports' pandas change only touches setup.py, which the Meson build does
# not use -- no patch is needed.
PANDAS_BUILD_NUMPY=2.3.4
source "$WS/scripts/lib/pywheel.sh"
do_configure() {
  qnx_py_venv "meson-python>=0.13.1" "meson>=1.2.1" "Cython~=3.0.5" "versioneer[toml]" "numpy==$PANDAS_BUILD_NUMPY" "patchelf>=0.11.0" || return 1
  local hostinc
  hostinc=$("$B/venv/bin/python" -c 'import numpy; print(numpy.get_include())') || return 1
  sed -i 's|^#define NPY_HAVE_ENDIAN_H 1$|/* #undef NPY_HAVE_ENDIAN_H */|' "$hostinc/numpy/_numpyconfig.h" || return 1
  grep -q '^#define NPY_HAVE_ENDIAN_H' "$hostinc/numpy/_numpyconfig.h" && { echo "NPY_HAVE_ENDIAN_H still set"; return 1; }
  qnx_py_crossfile
}
do_build() { qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" -Csetup-args="--native-file=$B/qnx-native.ini"; }
do_install() { qnx_py_install; }
