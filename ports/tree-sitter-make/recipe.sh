# tree-sitter grammar: make. Installs lib/tree-sitter/make.so (the parser
# Neovim and the tree-sitter library load). Version as in QNX's repository.
VERSION=1.1.1; GIT=https://github.com/tree-sitter-grammars/tree-sitter-make.git; REF=v1.1.1
BUILDSYS=custom
do_build() {
  local d="$S/src" f objs=()
  for f in "$d"/parser.c "$d"/scanner.c "$d"/scanner.cc; do [ -f "$f" ] && objs+=("$f"); done
  qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/make.so" "${objs[@]}"
}
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B/make.so" "$STAGE$PREFIX/lib/tree-sitter/"; }
