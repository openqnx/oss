# tree-sitter grammar: cpp (C++). Installs lib/tree-sitter/cpp.so (the parser
# Neovim and the tree-sitter library load). Version as in the QNX Everywhere image.
VERSION=0.23.4; GIT=https://github.com/tree-sitter/tree-sitter-cpp.git; REF=v0.23.4
BUILDSYS=custom
do_build() {
  local d="$S/src" f objs=()
  for f in "$d"/parser.c "$d"/scanner.c; do [ -f "$f" ] && objs+=("$f"); done
  qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/cpp.so" "${objs[@]}"
}
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B/cpp.so" "$STAGE$PREFIX/lib/tree-sitter/"; }
