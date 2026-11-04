# python-zstandard: Zstandard compression for Python (one-shot, streaming,
# dictionaries). Its C backend, cross compiled like netifaces
# (scripts/lib/pywheel.sh), linked against the ported zstd library
# (--system-zstd) instead of the copy bundled in the sdist. The optional cffi
# backend is not built: the build's requirement check (cffi>=2 on the build
# machine) is skipped (-x), and without cffi setup.py leaves that backend out.
VERSION=0.25.0
TARBALL=https://files.pythonhosted.org/packages/fd/aa/3e0508d5a5dd96529cdc5a97011299056e14c6505b678fd58938792794b1/zstandard-0.25.0.tar.gz
SHA256=7713e1179d162cf5c7906da876ec2ccb9c3a9dcbdffef0cc7f70c3667a205f0b
DEPS="python3 zstd"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77"; }
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" \
  qnx_py_wheel -x -C--build-option=--system-zstd
}
do_install() { qnx_py_install; }
