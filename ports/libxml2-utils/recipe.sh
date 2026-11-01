# xmllint and xmlcatalog, libxml2's command-line tools: parse, validate
# (DTD, RelaxNG, XML Schema), pretty-print and query XML, and maintain XML
# catalogs. The libxml2 component is built without its programs; this
# builds them from the same release and installs only the programs, which
# link that component's libxml2.so.2.
VERSION=2.13.5; GIT=https://gitlab.gnome.org/GNOME/libxml2.git; REF=v2.13.5
DEPS="libxml2 zlib xz"
# Same options as the libxml2 component except the Python bindings (not
# needed for the programs). No readline (xmllint --shell then reads plain
# lines): libxml2 2.13's CMakeLists links the programs with libhistory
# whenever it finds one, whatever LIBXML2_WITH_HISTORY says, so the probe
# results are preset. The programs are installed by hand, so they are
# linked without the build tree's RPATH.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DLIBXML2_WITH_PYTHON=OFF -DLIBXML2_WITH_TESTS=OFF
                   -DLIBXML2_WITH_PROGRAMS=ON -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=ON
                   -DLIBXML2_WITH_ZLIB=ON -DLIBXML2_WITH_READLINE=OFF -DLIBXML2_WITH_HISTORY=OFF
                   -DHAVE_LIBHISTORY=0 -DHAVE_LIBREADLINE=0 -DCMAKE_SKIP_BUILD_RPATH=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
do_build() { cmake --build "$B" -j"$(nproc)" --target xmllint xmlcatalog; }
# Their man pages are already in the libxml2 component.
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/xmllint" "$B/xmlcatalog" "$STAGE$PREFIX/bin/"; }
