# ml_dtypes: NumPy dtype extensions for machine learning (bfloat16, the
# float8 and float4/6 families, int2/int4/uint4), a C++ extension module built
# with Eigen (vendored in the source distribution) against the ported numpy.
# Built from the source distribution, which carries the Eigen copy the
# release was made with (the git tag has it as a submodule).
VERSION=0.6.0
TARBALL=https://files.pythonhosted.org/packages/12/72/307d7c4bd0600601c7133fba5cb78af7db968152951c1cd473abb1cda782/ml_dtypes-0.6.0.tar.gz
SHA256=5e60251d32ced5598972e4d5e06a2f044341f9291402551a3f6f0ec44f9299b0
DEPS="numpy"
RUNTIME_DEPS="numpy"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
# scikit-build-core with the build venv's numpy for the build requirement,
# whose headers are replaced by the *target* numpy's (as for pandas: the
# generated _numpyconfig.h differs per platform).
do_configure() {
  qnx_py_venv "scikit-build-core>=0.10" "numpy==2.3.4" || return 1
  local hostinc tgtinc="$STAGE$PREFIX/lib/python$QNX_PYV/site-packages/numpy/_core/include"
  hostinc=$("$B/venv/bin/python" -c 'import numpy; print(numpy.get_include())') || return 1
  [ -d "$tgtinc" ] || { echo "target numpy headers missing: $tgtinc"; return 1; }
  rm -rf "$hostinc" && cp -a "$tgtinc" "$hostinc"
}
# CMake is pointed at the QNX toolchain and the target's Python headers;
# FindPython would otherwise take the build machine's, and name the module
# with its SOABI (cpython-314-x86_64-linux-gnu) -- the target's is
# cpython-314.
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" \
    -Ccmake.define.CMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -Ccmake.define.Python_INCLUDE_DIR="$STAGE$PREFIX/include/python$QNX_PYV" \
    -Ccmake.define.Python_SOABI=cpython-${QNX_PYV/./} \
    -Ccmake.define.CMAKE_BUILD_TYPE=Release
}
do_install() { qnx_py_install; }
