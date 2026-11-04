# cffi: the C foreign function interface for Python -- calls C code from
# Python in ABI mode (dlopen, no compiler) or API mode (generated extension
# modules). Its backend _cffi_backend is a setuptools C extension on libffi,
# cross compiled like netifaces (scripts/lib/pywheel.sh); setup.py would ask
# pkg-config of the build machine, so libffi's flags are given here.
VERSION=2.1.1
TARBALL=https://files.pythonhosted.org/packages/9e/ef/008a1939e372c06329a3fce4279c02f328488f3526744906eeec3da7ad5f/cffi-2.1.1.tar.gz
SHA256=dd31f52ea1086513bb9df30f8fcee9b8918323ae067a3d5b78bc826a000712be
DEPS="python3 libffi python3-cparser"
RUNTIME_DEPS="python3 python3-cparser"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77"; }
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" qnx_py_wheel
}
do_install() { qnx_py_install; }
