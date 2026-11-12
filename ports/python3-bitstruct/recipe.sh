# bitstruct: packs and unpacks values at bit granularity ("u1u3u4s16" format
# strings), the way struct does bytes; cantools encodes CAN signals with it.
# Built with its C extension (bitstruct.c), cross compiled like msgpack
# (scripts/lib/pywheel.sh); the pure-Python implementation stays in the
# package (the default `import bitstruct`).
VERSION=8.24.0
TARBALL=https://files.pythonhosted.org/packages/e0/15/ed0024e96ea5d90075874fb3288a93b45f275d97be3e21becac076273c78/bitstruct-8.24.0.tar.gz
SHA256=c72d8bd3b1762a5bcc9fde16bfa4d52800a4bc8d79f81bc7ee0e0bca5b5e901e
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=78.1.1" wheel; }
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
do_install() { qnx_py_install; }
