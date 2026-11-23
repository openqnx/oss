VERSION=0.27.2; GIT=https://github.com/tree-sitter/tree-sitter.git; REF=v0.27.2
BUILDSYS=cmake
# 0.26 moved the library's CMakeLists.txt from lib/ to the top of the tree.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DTREE_SITTER_FEATURE_WASM=OFF )
