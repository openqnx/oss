# protobuf (Python): Protocol Buffers for Python -- messages, the text and
# JSON formats, descriptors -- with its C extension google._upb._message (the
# upb runtime, bundled in the source distribution), cross compiled like
# cffi's (scripts/lib/pywheel.sh). Version 5.29.3 is the Python release of
# the ported protobuf 29.3 (ports/protobuf): code that protoc 29.3 generates
# needs exactly this runtime.
VERSION=5.29.3
TARBALL=https://files.pythonhosted.org/packages/f7/d1/e0a911544ca9993e0f17ce6d3cc0932752356c1b0a834397f28e63479344/protobuf-5.29.3.tar.gz
SHA256=5da0f41edaf117bde316404bad1a486cb4ededf8e4a54891296f648e8e076620
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
do_install() { qnx_py_install; }
