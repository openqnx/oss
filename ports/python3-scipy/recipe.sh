# SciPy: scientific computing for Python -- linear algebra, optimisation,
# integration, interpolation, FFT, signal and image processing, sparse
# matrices, statistics, special functions. Cross built like numpy
# (meson-python with a cross file, scripts/lib/pywheel.sh) against the
# ported OpenBLAS, whose LAPACK is the C translation (C_LAPACK), so neither
# needs a Fortran compiler.
# SciPy 1.18 has turned all of its own Fortran into C except the deprecated
# scipy.odr: -D_without-fortran=true builds everything else and leaves
# scipy.odr out (no Fortran cross compiler exists for QNX SDP 8.0).
# numpy's and Pythran's C/C++ headers are the target's staged copies (the
# numpy-include-dir and pythran-include-dir cross properties); the build
# venv's Pythran (the same version as the pythran port) translates SciPy's
# Pythran kernels to C++.
VERSION=1.18.1
TARBALL=https://files.pythonhosted.org/packages/7e/74/66de6258867beb2ef08f35f9f2ac017a52cacd5081714d239ff1a442d458/scipy-1.18.1.tar.gz
SHA256=52c4b7422442aba924d03ad4019852b08a92e64ea187b933135687bfe2747307
DEPS="numpy openblas pythran pybind11"
RUNTIME_DEPS="numpy"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() {
  local sp="$STAGE$PREFIX/lib/python$QNX_PYV/site-packages"
  qnx_py_venv "meson-python>=0.15.0,<0.22.0" "meson>=1.5" "Cython>=3.2.0,<3.3.0" "pybind11>=2.13.2,<3.1.0" \
              "pythran==$(sed -n 's/^VERSION=//p' "$WS/ports/pythran/recipe.sh")" "numpy==$(sed -n 's/^VERSION=\([^;]*\).*/\1/p' "$WS/ports/numpy/recipe.sh")" \
              "patchelf>=0.11.0" || return 1
  [ -d "$sp/numpy/_core/include" ] && [ -d "$sp/pythran/pythonic" ] || { echo "staged numpy/pythran headers missing"; return 1; }
  qnx_py_crossfile "numpy-include-dir = '$sp/numpy/_core/include'" "pythran-include-dir = '$sp/pythran'" || return 1
  echo "pybind11-config = '$B/venv/bin/pybind11-config'" >> "$B/qnx-cross.ini"
  # dependency('numpy'): numpy's own numpy.pc names ${pcfiledir}, which the
  # cross pkg-config would prefix with the sysroot a second time; this one is
  # relative to the sysroot like every staged .pc.
  mkdir -p "$B/pc" && cat > "$B/pc/numpy.pc" <<PC || return 1
prefix=$PREFIX/lib/python$QNX_PYV/site-packages/numpy/_core
includedir=\${prefix}/include
Name: numpy
Description: NumPy (staged package)
Version: $(sed -n 's/^VERSION=\([^;]*\).*/\1/p' "$WS/ports/numpy/recipe.sh")
Cflags: -I\${includedir}
PC
  sed -i "s#^pkg_config_libdir = \['\([^']*\)'\]#pkg_config_libdir = ['\1', '$B/pc']#" "$B/qnx-cross.ini"
  echo "pythran = '$B/venv/bin/pythran'" >> "$B/qnx-native.ini"
}
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" \
    -Csetup-args="--native-file=$B/qnx-native.ini" -Csetup-args="-D_without-fortran=true" \
    -Csetup-args="-Dblas=openblas" -Csetup-args="-Dlapack=openblas" -Csetup-args="--wrap-mode=nodownload"
}
# scipy.show_config() records the build tools: the build venv's cython is
# named without the build tree.
do_install() {
  qnx_py_install
  sed -i "s#$B/venv/bin/##g" "$STAGE$PREFIX/lib/python$QNX_PYV/site-packages/scipy/__config__.py"
}
