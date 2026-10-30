# tree-sitter grammar: python. Installs lib/tree-sitter/python.so (the parser
# Neovim and the tree-sitter library load). Version as in QNX's repository.
VERSION=0.23.6; GIT=https://github.com/tree-sitter/tree-sitter-python.git; REF=v0.23.6
BUILDSYS=custom
do_build() {
  local d="$S/src" f objs=()
  for f in "$d"/parser.c "$d"/scanner.c "$d"/scanner.cc; do [ -f "$f" ] && objs+=("$f"); done
  qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/python.so" "${objs[@]}"
}
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B/python.so" "$STAGE$PREFIX/lib/tree-sitter/"; }
