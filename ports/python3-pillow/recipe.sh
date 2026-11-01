# Pillow: the Python imaging library (image file formats, drawing, filters,
# fonts). A setuptools C extension, cross compiled like numpy
# (scripts/lib/pywheel.sh). platform-guessing=disable: Pillow's setup.py
# otherwise searches the build machine's /usr/lib and /usr/include; with it
# off, libraries are found through pkg-config (the staging tree) and the
# target's sysconfig flags. Required (build fails without): zlib, JPEG,
# FreeType; also TIFF, WebP, LCMS2, OpenJPEG and libraqm (complex text
# layout) from the ported libraries. Not built: imagequant, AVIF, XCB.
VERSION=13.0.0
TARBALL=https://files.pythonhosted.org/packages/ce/a4/9cc30690a394a8305e8be4c9036aa16d0dfddadd09079860b582f562496d/pillow-13.0.0.tar.gz
SHA256=cdcd1ba258178f7356d74170c9a81ba4eab6a5d55748bc402a5a2a6fe6be3510
DEPS="python3 zlib libjpeg-turbo freetype libtiff libwebp lcms2 openjpeg libraqm"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77" "pybind11"; }
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  CXXFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  LDCXXSHARED="q++ -V${QCC_TARGET}_cxx -shared" \
  qnx_py_wheel -Cplatform-guessing=disable -Czlib=enable -Cjpeg=enable -Cfreetype=enable \
    -Ctiff=enable -Cwebp=enable -Clcms=enable -Cjpeg2000=enable -Craqm=enable \
    -Cimagequant=disable -Cavif=disable -Cxcb=disable
}
do_install() { qnx_py_install; }
