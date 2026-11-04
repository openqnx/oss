# msgpack: MessagePack, a compact binary serialisation format, for Python.
# Built with its C extension (msgpack._cmsgpack, Cython generated on the
# build machine), cross compiled like netifaces (scripts/lib/pywheel.sh);
# the pure-Python fallback stays in the package.
VERSION=1.2.4
TARBALL=https://files.pythonhosted.org/packages/cc/52/1ec7ead2c7528bf3d1db73e886fa4df10f9e6f0eb7854a448b7090cfcef7/msgpack-1.2.4.tar.gz
SHA256=475c44e12f2a5de766a8448783f149c9b33f3f49b88953c6cd739a77d93c732c
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=78.1.1" "Cython>=3.1"; }
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  CXXFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  LDCXXSHARED="q++ -V${QCC_TARGET}_cxx -shared" qnx_py_wheel
}
do_install() { qnx_py_install; }
