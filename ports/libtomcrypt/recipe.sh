VERSION=1.18.2
TARBALL=https://github.com/libtom/libtomcrypt/releases/download/v1.18.2/crypt-1.18.2.tar.xz
SHA256=96ad4c3b8336050993c5bc2cf6c057484f2b0f9f763448151567fbab5e767b84
DEPS="libtommath"
BUILDSYS=custom
# makefile.shared needs a libtool that knows the QNX cross compiler; the
# plain makefile.unix (static archive only) is portable make. Its objects are
# compiled position-independent and then linked into the shared library
# libtool would have made: libtomcrypt.so.1.0.1, soname libtomcrypt.so.1
# (-version-info 1:1, as in makefile_include.mk). USE_LTM/LTM_DESC builds the
# libtommath math descriptor (ltm_desc) for RSA, DSA, DH and ECC; the library
# then needs libtommath.so.1.
_ltcmake() { make -C "$B" -f makefile.unix -j6 CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" AR="nto${QNX_CPU}-ar" RANLIB="nto${QNX_CPU}-ranlib" \
  CFLAGS="-O2 -fPIC -DUSE_LTM -DLTM_DESC -I$STAGE$PREFIX/include" EXTRALIBS="-L$STAGE$PREFIX/lib -ltommath" PREFIX="$PREFIX" "$@"; }
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _ltcmake libtomcrypt.a && cd "$B" && \
  qcc -V$QCC_TARGET -shared -Wl,-soname,libtomcrypt.so.1 -o libtomcrypt.so.1.0.1 \
    -Wl,--whole-archive libtomcrypt.a -Wl,--no-whole-archive -L"$STAGE$PREFIX/lib" -ltommath; }
# Shared library only (as libtommath): the static archive is a build step.
do_install() { _ltcmake DESTDIR="$STAGE" install && rm -f "$STAGE$PREFIX/lib/libtomcrypt.a" && \
  sed -i 's,^Libs:.*,&\nLibs.private: -ltommath,; s,^Cflags:.*,Cflags: -I${includedir} -DUSE_LTM -DLTM_DESC,' "$STAGE$PREFIX/lib/pkgconfig/libtomcrypt.pc" && \
  install -m755 "$B/libtomcrypt.so.1.0.1" "$STAGE$PREFIX/lib/" && \
  ln -sf libtomcrypt.so.1.0.1 "$STAGE$PREFIX/lib/libtomcrypt.so.1" && ln -sf libtomcrypt.so.1 "$STAGE$PREFIX/lib/libtomcrypt.so"; }
