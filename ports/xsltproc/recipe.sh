# xsltproc, libxslt's command-line XSLT processor -- what xmlto, asciidoc's
# a2x and many projects' documentation builds run to apply the DocBook XSL
# stylesheets. The libxslt component is built without its programs; this
# builds libxslt's own xsltproc from the same release and installs only the
# program, which links the libxslt and libexslt libraries of that component.
VERSION=1.1.42; GIT=https://gitlab.gnome.org/GNOME/libxslt.git; REF=v1.1.42
DEPS="libxslt libxml2"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DLIBXSLT_WITH_PYTHON=OFF -DLIBXSLT_WITH_TESTS=OFF
                   -DLIBXSLT_WITH_PROGRAMS=ON -DLIBXSLT_WITH_CRYPTO=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DCMAKE_SKIP_BUILD_RPATH=ON )
# Same options as the libxslt component (so libxslt.so.1/libexslt.so.0 of
# this build are the same libraries); only xsltproc and its man page are
# installed, by hand, hence linked without the build tree's RPATH.
do_build() { cmake --build "$B" -j"$(nproc)" --target xsltproc; }
do_install() {
  install -d "$STAGE$PREFIX/bin" "$STAGE$PREFIX/share/man/man1" && \
  install -m755 "$B/xsltproc" "$STAGE$PREFIX/bin/" && \
  install -m644 "$S/doc/xsltproc.1" "$STAGE$PREFIX/share/man/man1/"
}
