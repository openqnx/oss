VERSION=2.3.3; GIT=https://github.com/pandas-dev/pandas.git; REF=v2.3.3
DEPS="numpy python3-dateutil python3-pytz python3-tzdata"
BUILDSYS=custom
# pandas 2.3.3: the first release line that supports Python 3.14 (the
# target's). Cross built like numpy (scripts/lib/pywheel.sh). pandas finds
# numpy's C headers by asking the build-host numpy for get_include(), and
# numpy's generated _numpyconfig.h differs per platform (long double size), so
# the host numpy in the build venv is given the *target* numpy's headers.
# qnx-ports' pandas change only touches setup.py, which the Meson build does
# not use -- no patch is needed.
source "$WS/scripts/lib/pywheel.sh"
do_configure() {
  qnx_py_venv "meson-python>=0.13.1" "meson>=1.2.1" "Cython~=3.0.5" "versioneer[toml]" "numpy==2.3.4" "patchelf>=0.11.0" || return 1
  local hostinc tgtinc="$STAGE$PREFIX/lib/python$QNX_PYV/site-packages/numpy/_core/include"
  hostinc=$("$B/venv/bin/python" -c 'import numpy; print(numpy.get_include())') || return 1
  [ -d "$tgtinc" ] || { echo "target numpy headers missing: $tgtinc"; return 1; }
  rm -rf "$hostinc" && cp -a "$tgtinc" "$hostinc"
  qnx_py_crossfile
}
do_build() { qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" -Csetup-args="--native-file=$B/qnx-native.ini"; }
do_install() { qnx_py_install; }
