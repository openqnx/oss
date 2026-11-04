# MarkupSafe: a str subclass that escapes text for HTML/XML (Jinja2's
# autoescaping). Built with its C speedups (markupsafe._speedups), a
# setuptools C extension cross compiled like netifaces (scripts/lib/pywheel.sh);
# the pure-Python fallback stays in the package.
VERSION=3.0.4
TARBALL=https://files.pythonhosted.org/packages/38/9b/e422a865e1d5d57d0e509b4e0bf1c1a70a7f6382c29a5aa428df994c8bc8/markupsafe-3.0.4.tar.gz
SHA256=2e9ad7dd851bf45fab9f75cbff4cb493fee9979e8d8c7c9c3ee119022518edd6
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77"; }
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  CIBUILDWHEEL=1 CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
do_install() { qnx_py_install; }
