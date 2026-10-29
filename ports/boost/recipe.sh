VERSION=1.86.0
# Also linked at run time (found by scripts/gen-dependency-index): xz
DEPS="zlib bzip2 xz"
TARBALL=https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz
SHA256=2575e74ffc3ef1cd0babac2c1ee8bdb5782a0ee672b1912da40e5b4b591ca01f
BUILDSYS=custom
# Build every compiled Boost library except python, which would need a target
# Python interpreter and headers to link against. Leaving BOOST_LIBS empty makes
# b2 build its default set (everything), rather than the --with-<lib> subset the
# automotive middleware alone needs -- a consumer reaching for boost_json,
# boost_url, boost_context or boost_locale should find them staged.
BOOST_LIBS=""

# NOTE: b2 5.2.1 (bundled with Boost 1.86) segfaults during configuration of its
# own "qcc" toolset, with or without a user-config. The QNX cross-GCC driver
# behind qcc works fine when declared as a *named gcc toolset*, so use that.
_b2_args() {
  B2_ARGS=( -q -j"$(nproc)"
    --build-dir="$B" --user-config="$B/user-config.jam"
    toolset=gcc-qnx target-os=qnxnto
    # Boost.Context picks its assembly from <abi>, and b2 derives the default
    # from the *build host's* platform -- so a cross build to aarch64 asks for
    # sysv/arm64 assembly, which does not exist, and the whole build dies with
    # "No best alternative for libs/context/build/asm_sources".
    "abi=$( [ "$QNX_ARCH" = aarch64le ] && echo aapcs || echo sysv )"
    variant=release link=shared runtime-link=shared threading=multi
    cxxstd=17 define=BOOST_SYSTEM_NO_DEPRECATED
    # Boost.Iostreams probes for zlib/bzip2 in the compiler's default paths, so
    # without these it linked the SDP's prebuilt zlib -- whose soname is
    # libz.so.2 against upstream's libz.so.1 -- and libboost_iostreams then
    # loaded the SDP copy on the target. Same for bzip2.
    "-sZLIB_INCLUDE=$STAGE$PREFIX/include"  "-sZLIB_LIBPATH=$STAGE$PREFIX/lib"
    "-sBZIP2_INCLUDE=$STAGE$PREFIX/include" "-sBZIP2_LIBPATH=$STAGE$PREFIX/lib"
    --prefix="$STAGE$PREFIX"
    --includedir="$STAGE$PREFIX/include"
    --libdir="$STAGE$PREFIX/lib"
    --without-python )
  for l in $BOOST_LIBS; do B2_ARGS+=( "--with-$l" ); done
}

do_configure() {
  cd "$S"
  [ -x ./b2 ] || ./bootstrap.sh --with-toolset=gcc --without-libraries=python
  mkdir -p "$B"
  cat > "$B/user-config.jam" <<JAM
using gcc : qnx : $QNX_HOST/usr/bin/$QNX_TRIPLE-g++ :
  <cxxflags>-D_QNX_SOURCE
  <cxxflags>-Wno-ignored-attributes
  <linkflags>-lsocket
  <linkflags>-lm
  <archiver>$QNX_HOST/usr/bin/nto$QNX_CPU-ar
  <ranlib>$QNX_HOST/usr/bin/nto$QNX_CPU-ranlib
  ;
JAM
}

do_build()   { cd "$S"; _b2_args; ./b2 "${B2_ARGS[@]}"; }
do_install() { cd "$S"; _b2_args; ./b2 "${B2_ARGS[@]}" install; }
