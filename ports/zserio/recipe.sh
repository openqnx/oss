# zserio: serialization framework for binary data (a schema language, a code
# generator and per-language runtimes). This is the C++ runtime library:
# bit stream reader/writer, bit buffers, arrays and packing contexts,
# varints, JSON encoding/decoding, type info and reflection, the pmr
# allocators, and the interfaces generated services, pub/sub and SQLite
# databases implement.
# The code generator is a Java program that runs on the build machine
# (zserio.jar, from the release or `pip install zserio`); it is not part of
# this package, and code generated with the same zserio release compiles
# against these headers.
VERSION=2.20.0; GIT=https://github.com/ndsev/zserio.git; REF=v2.20.0
SUBDIR=compiler/extensions/cpp/runtime/src
# Upstream builds the runtime as a static library and has no install rules
# for it (its own "install" copies the sources). The archive is built with
# -fPIC so that it can go into shared libraries of generated code.
CMAKE_ARGS_EXTRA=( -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
do_install() {
  local inc="$STAGE$PREFIX/include" f
  mkdir -p "$inc" "$STAGE$PREFIX/lib" || return 1
  ( cd "$S" && find zserio -name '*.h' ) | while read -r f; do
    install -Dm644 "$S/$f" "$inc/$f" || exit 1; done || return 1
  install -m644 "$B/libZserioCppRuntime.a" "$STAGE$PREFIX/lib/"
}
