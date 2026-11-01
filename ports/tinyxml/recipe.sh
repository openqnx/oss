VERSION=2.6.2
TARBALL=https://downloads.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.tar.gz
SHA256=15bdfdcec58a7da30adc87ac2b078e4417dbe5392f3afb719f9ba6d062645593
BUILDSYS=custom
# The original TinyXML (not TinyXML-2, which is the tinyxml2 component and
# installs tinyxml2.h / libtinyxml2: the two coexist). Upstream ships only a
# Makefile for its test program, so the library is built here the way the
# Linux distributions build it: libtinyxml.so.0 with the STL interface
# (patches/0001 makes the header say so), plus tinyxml.pc. patches/0002 fixes
# the two published parser bugs; upstream has been inactive since 2011.
TX_SRCS="tinyxml.cpp tinystr.cpp tinyxmlerror.cpp tinyxmlparser.cpp"
do_build() {
  local f objs=
  for f in $TX_SRCS; do
    q++ -V${QCC_TARGET}_cxx -O2 -fPIC -DNDEBUG -D_QNX_SOURCE -DTIXML_USE_STL -c "$S/$f" -o "$B/${f%.cpp}.o" || return 1
    objs="$objs $B/${f%.cpp}.o"
  done
  q++ -V${QCC_TARGET}_cxx -shared -Wl,-soname,libtinyxml.so.0 -o "$B/libtinyxml.so.0.$VERSION" $objs -lm || return 1
  # upstream's test program, linked only to prove the library links (not installed)
  q++ -V${QCC_TARGET}_cxx -O2 -D_QNX_SOURCE -DTIXML_USE_STL -I"$S" "$S/xmltest.cpp" -o "$B/xmltest" -L"$B" -l:libtinyxml.so.0.$VERSION
}
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" &&
  install -m755 "$B/libtinyxml.so.0.$VERSION" "$L/" &&
  ln -sf "libtinyxml.so.0.$VERSION" "$L/libtinyxml.so.0" && ln -sf libtinyxml.so.0 "$L/libtinyxml.so" &&
  install -m644 "$S/tinyxml.h" "$S/tinystr.h" "$STAGE$PREFIX/include/" &&
  cat > "$L/pkgconfig/tinyxml.pc" <<PC
prefix=$PREFIX
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: TinyXml
Description: simple, small, C++ XML parser
Version: $VERSION
Libs: -L\${libdir} -ltinyxml
Cflags: -I\${includedir} -DTIXML_USE_STL=YES
PC
}
