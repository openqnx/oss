# tree-sitter grammars: markdown and markdown_inline (one repository, two
# parsers). Installs lib/tree-sitter/{markdown,markdown_inline}.so. Version as
# in the QNX Everywhere 8.0.4r1 image.
VERSION=0.5.1; GIT=https://github.com/tree-sitter-grammars/tree-sitter-markdown.git; REF=v0.5.1
BUILDSYS=custom
do_build() {
  local g d
  for g in markdown markdown-inline; do
    d="$S/tree-sitter-$g/src"
    qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/${g/-/_}.so" "$d/parser.c" "$d/scanner.c" || return 1
  done
}
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B"/markdown.so "$B"/markdown_inline.so "$STAGE$PREFIX/lib/tree-sitter/"; }
