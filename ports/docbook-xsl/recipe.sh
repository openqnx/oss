# docbook-xsl: the DocBook XSL stylesheets (DocBook XML to HTML, man pages,
# FO), as xsltproc applies them -- asciidoc's and many projects' man pages.
# Data only, under share/xml/docbook/xsl-stylesheets-<version>.
VERSION=1.79.2
TARBALL=https://github.com/docbook/xslt10-stylesheets/releases/download/release%2F1.79.2/docbook-xsl-1.79.2.tar.bz2
FETCH_NAME=docbook-xsl-1.79.2.tar.bz2
SHA256=316524ea444e53208a2fb90eeb676af755da96e1417835ba5f5eb719c81fa371
DEPS=""
RUNTIME_DEPS="libxslt"
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_configure() { :; }
do_build() { :; }
do_install() {
  local d="$STAGE$PREFIX/share/xml/docbook/xsl-stylesheets-$VERSION"; mkdir -p "$d"
  local s; s=$(ls -d "$S"/docbook-xsl-* | head -n 1)
  ( cd "$s" && cp -a VERSION VERSION.xsl catalog.xml common html manpages fo lib highlighting params xhtml xhtml-1_1 xhtml5 "$d/" )
}
