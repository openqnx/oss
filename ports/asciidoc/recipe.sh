# AsciiDoc.py 10.2.1: the asciidoc text-to-HTML/DocBook converter and a2x
# (DocBook to man pages, HTML, ... through xsltproc and the DocBook XSL
# stylesheets). Pure Python: the published wheel is platform independent and
# is installed as released (see scripts/lib/pywheel.sh).
VERSION=10.2.1
TARBALL=https://files.pythonhosted.org/packages/75/1f/87941eaa96e86aa22086064f67e4187e2710fb76c147312979ea29278dac/asciidoc-10.2.1-py2.py3-none-any.whl
SHA256=3f277a636b617c9ce7e0b87bcaea51f144500e9a5c8a6488421ee24594850d40
DEPS="python3"
# asciidoc itself needs only python3; a2x runs xsltproc with the DocBook XSL
# stylesheets (and xmllint to validate, unless -L).
RUNTIME_DEPS="python3 docbook-xsl xsltproc"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
# The wheel's two console scripts (entry_points.txt), as pip would write
# them, for the target's python3.
do_install() {
  qnx_py_install_pure || return 1
  local m
  install -d "$STAGE$PREFIX/bin"
  for m in asciidoc a2x; do
    printf '#!%s/bin/python3\nimport sys\nfrom asciidoc.%s import cli\nif __name__ == "__main__":\n    sys.exit(cli())\n' \
      "$PREFIX" "$m" > "$STAGE$PREFIX/bin/$m" && chmod 755 "$STAGE$PREFIX/bin/$m" || return 1
  done
}
