# Cython: the Python-to-C compiler for extension modules. Built from the
# source distribution with its compiled modules (the compiler's scanner,
# parser and code generator, Cython.Runtime.refnanny, ...), cross compiled
# for the target's python3 (scripts/lib/pywheel.sh); the cython, cythonize
# and cygdb commands are written as launchers.
VERSION=3.3.1
TARBALL=https://files.pythonhosted.org/packages/99/ca/18e324972e5cac914816fdd7bb03e19a455b90039c277a538f7781b50c85/cython-3.3.1.tar.gz
SHA256=e0e45989e821bef6aa9ae15c7d583ab5e055f7406c4b3624e49e06d24e90375a
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77"; }
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
