# Helpers for recipes that cross build a Python package into a wheel and
# install it into the target's site-packages. Sourced by recipe.sh files:
#
#   source "$WS/scripts/lib/pywheel.sh"
#   do_configure() { qnx_py_venv "meson-python>=0.18" "Cython>=3.0.6" && qnx_py_crossfile; }
#   do_build()     { qnx_py_wheel -Csetup-args="-Dfoo=bar"; }
#   do_install()   { qnx_py_install; }
#
# How it works (the approach qnx-ports uses for numpy):
#  - a venv of the build host's Python of the *same version* as the target's
#    (hosttools/bin/python$QNX_PYV, built by scripts/build-host-tools.sh);
#  - the target's sysconfig data (staged by the python3 recipe) is copied into
#    it and selected with _PYTHON_SYSCONFIGDATA_NAME, so extension modules get
#    QNX's suffixes and flags;
#  - Meson-based packages get a cross file for the QNX compilers.
QNX_PYV=${QNX_PYV:-3.14}

qnx_py_venv() {
  local v="$B/venv"
  if [ ! -x "$v/bin/python" ]; then
    "$WS/hosttools/bin/python$QNX_PYV" -m venv "$v" || return 1
    cp "$STAGE$PREFIX/lib/python$QNX_PYV/_sysconfigdata__qnx_.py" "$v/lib/python$QNX_PYV/site-packages/" || return 1
  fi
  "$v/bin/pip" install -q --disable-pip-version-check build wheel "$@"
}

# qnx_py_crossfile [extra [properties] lines...] -> $B/qnx-cross.ini
qnx_py_crossfile() {
  local py="$STAGE$PREFIX/include/python$QNX_PYV"
  {
    echo "[host_machine]"
    echo "system = 'qnx'"; echo "cpu_family = '$QNX_CPU'"; echo "cpu = '$QNX_CPU'"; echo "endian = 'little'"
    echo "[properties]"
    echo "pkg_config_libdir = ['$STAGE$PREFIX/lib/pkgconfig']"
    echo "sys_root = '$STAGE'"
    for l in "$@"; do echo "$l"; done
    echo "[built-in options]"
    echo "c_args = ['-D_QNX_SOURCE', '-I$py']"
    echo "cpp_args = ['-D_QNX_SOURCE', '-I$py']"
    # libm too: Meson's checks for math functions (numpy's "sin" with
    # dependency -lm) otherwise link without it and fail with "DSO missing".
    echo "c_link_args = ['-L$STAGE$PREFIX/lib', '-lpython$QNX_PYV', '-lm']"
    echo "cpp_link_args = ['-L$STAGE$PREFIX/lib', '-lpython$QNX_PYV', '-lm']"
    echo "[binaries]"
    # The GCC driver qcc runs, named so that Meson recognises it (it does not
    # know qcc), as qnx-ports' numpy cross file does.
    echo "c = ['$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc']"
    echo "cpp = ['$QNX_HOST/usr/bin/$QNX_TRIPLE-g++']"
    echo "ar = '$QNX_HOST/usr/bin/nto${QNX_CPU}-ar'"
    echo "strip = '$QNX_HOST/usr/bin/nto${QNX_CPU}-strip'"
    echo "pkg-config = 'pkg-config'"
  } > "$B/qnx-cross.ini"
  # Cython runs on the build machine: Meson looks for it in the native file.
  { echo "[binaries]"; echo "cython = '$B/venv/bin/cython'"; } > "$B/qnx-native.ini"
}

# qnx_py_wheel [extra python -m build args...] -> $B/wheel/*.whl
# The venv comes first in PATH: Meson runs a project's helper scripts with the
# python3 it finds there, and pandas' generate_version.py needs versioneer,
# which only the venv has.
qnx_py_wheel() {
  rm -rf "$B/wheel" && mkdir -p "$B/wheel" && cd "$S" && \
  PATH="$B/venv/bin:$PATH" CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" LDSHARED="qcc -V$QCC_TARGET -shared" \
  _PYTHON_SYSCONFIGDATA_NAME=_sysconfigdata__qnx_ _PYTHON_HOST_PLATFORM="qnx_8_0_0_$QNX_CPU" \
  "$B/venv/bin/python" -m build --wheel --no-isolation --outdir "$B/wheel" "$@"
}

qnx_py_install() {
  local sp="$STAGE$PREFIX/lib/python$QNX_PYV/site-packages" w
  mkdir -p "$sp"
  for w in "$B"/wheel/*.whl; do
    [ -e "$w" ] || { echo "no wheel in $B/wheel"; return 1; }
    "$WS/hosttools/bin/python$QNX_PYV" -m zipfile -e "$w" "$sp" || return 1
  done
}

# Pure-Python packages: the published py3-none-any wheel is the same on every
# platform, so it is downloaded (checksum pinned in the recipe as SHA256) and
# unpacked. Recipe: TARBALL=<wheel url>, SHA256=..., BUILDSYS=custom, and
#   do_fetch() { qnx_py_fetch_wheel; }  do_build() { :; }  do_install() { qnx_py_install_pure; }
qnx_py_fetch_wheel() {
  local w="$WS/dist/$(basename "$TARBALL")"
  mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$w" ] || curl -fsSL -o "$w" "$TARBALL" || return 1
  echo "$SHA256  $w" | sha256sum -c --status || { echo "checksum mismatch: $w"; rm -f "$w"; return 1; }
  cp "$w" "$SRCROOT/"
}
qnx_py_install_pure() {
  local sp="$STAGE$PREFIX/lib/python$QNX_PYV/site-packages" w
  mkdir -p "$sp"
  for w in "$SRCROOT"/*.whl; do "$WS/hosttools/bin/python$QNX_PYV" -m zipfile -e "$w" "$sp" || return 1; done
}
