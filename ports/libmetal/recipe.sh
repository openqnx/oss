VERSION=2024.10.0; GIT=https://github.com/OpenAMP/libmetal.git; REF=v2024.10.0
# libmetal builds with -Werror, and the QNX mutex stubs from the QNX patch
# leave parameters unused.
CMAKE_ARGS_EXTRA=( "-DCMAKE_C_FLAGS=-isystem $STAGE$PREFIX/include -Wno-error=unused-parameter" -DCMAKE_INSTALL_INCLUDEDIR=include -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_BINDIR=bin -DWITH_DOC=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_DEFAULT_LOGGER=ON )
