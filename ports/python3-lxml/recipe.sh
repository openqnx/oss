# lxml: the ElementTree API on libxml2 and libxslt (XML/HTML parsing,
# XPath, XSLT, XML Schema/RelaxNG validation, lxml.objectify). A setuptools
# C extension, cross compiled like Pillow (scripts/lib/pywheel.sh), against
# the ported libxml2, libxslt and libexslt (dynamically linked, found with
# pkg-config in staging). The source distribution carries the C files
# Cython generated; the build still asks for Cython (pyproject.toml), which
# the build venv provides.
VERSION=6.1.3
TARBALL=https://files.pythonhosted.org/packages/23/ad/28ecd7cb894d172f3c9c80a075eeeb2017ac62e3632cee05a5f9493547eb/lxml-6.1.3.tar.gz
SHA256=45222d94ddd511536f3b2f7d9deae3b2339b4ce0f075f1ca25703b07cad9dd21
DEPS="python3 libxml2 libxslt zlib"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77" "Cython>=3.2.9,<3.3"; }
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  qnx_py_wheel
}
do_install() { qnx_py_install; }
