# tree-sitter grammar: query. Installs lib/tree-sitter/query.so (the parser
# Neovim and the tree-sitter library load). Version as in the QNX Everywhere image.
VERSION=0.7.0; GIT=https://github.com/tree-sitter-grammars/tree-sitter-query.git; REF=v0.7.0
BUILDSYS=custom
do_build() {
  local d="$S/src" f objs=()
  for f in "$d"/parser.c "$d"/scanner.c; do [ -f "$f" ] && objs+=("$f"); done
  qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/query.so" "${objs[@]}"
}
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B/query.so" "$STAGE$PREFIX/lib/tree-sitter/"; }
