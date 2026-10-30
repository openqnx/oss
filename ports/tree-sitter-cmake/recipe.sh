# tree-sitter grammar: cmake. Installs lib/tree-sitter/cmake.so (the parser
# Neovim and the tree-sitter library load). Version as in QNX's repository.
VERSION=0.7.2; GIT=https://github.com/uyha/tree-sitter-cmake.git; REF=v0.7.2
BUILDSYS=custom
do_build() {
  local d="$S/src" f objs=()
  for f in "$d"/parser.c "$d"/scanner.c "$d"/scanner.cc; do [ -f "$f" ] && objs+=("$f"); done
  qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/cmake.so" "${objs[@]}"
}
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B/cmake.so" "$STAGE$PREFIX/lib/tree-sitter/"; }
