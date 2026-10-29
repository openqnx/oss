# tree-sitter grammar: c. Installs lib/tree-sitter/c.so (the parser
# Neovim and the tree-sitter library load). Version as in the QNX Everywhere image.
VERSION=0.24.1; GIT=https://github.com/tree-sitter/tree-sitter-c.git; REF=v0.24.1
BUILDSYS=custom
do_build() {
  local d="$S/src" f objs=()
  for f in "$d"/parser.c "$d"/scanner.c; do [ -f "$f" ] && objs+=("$f"); done
  qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/c.so" "${objs[@]}"
}
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B/c.so" "$STAGE$PREFIX/lib/tree-sitter/"; }
