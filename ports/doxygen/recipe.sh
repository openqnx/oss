# doxygen: documentation generator for C, C++ and more (HTML, XML, LaTeX, man
# pages from annotated sources). No doxywizard (needs Qt widgets), no clang
# assisted parsing; its lexers are generated with the build machine's flex/bison.
VERSION=1.17.0; GIT=https://github.com/doxygen/doxygen.git; REF=Release_1_17_0
DEPS="libiconv"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -Dbuild_wizard=OFF -Dbuild_search=OFF -Dbuild_doc=OFF -Duse_libclang=OFF -Duse_sys_sqlite3=OFF )
