# tree-sitter grammar: vimdoc. Installs lib/tree-sitter/vimdoc.so (the parser
# Neovim and the tree-sitter library load). Version as in the QNX Everywhere image.
VERSION=4.0.0; GIT=https://github.com/neovim/tree-sitter-vimdoc.git; REF=v4.0.0
BUILDSYS=custom
do_build() {
  local d="$S/src" f objs=()
  for f in "$d"/parser.c "$d"/scanner.c; do [ -f "$f" ] && objs+=("$f"); done
  qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/vimdoc.so" "${objs[@]}"
}
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B/vimdoc.so" "$STAGE$PREFIX/lib/tree-sitter/"; }
