VERSION=3.18.0
TARBALL=https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" "$WS/dist" && local z="$WS/dist/FreeImage3180.zip"; [ -f "$z" ] || curl -fsSL -o "$z" "$TARBALL"; \
  cd "$SRCROOT" && unzip -q "$z" && shopt -s dotglob && mv FreeImage/* . && rmdir FreeImage; }
# Makefile.gnu appends its include paths with "CFLAGS += $(INCLUDE)", which a
# command-line CFLAGS would override; pass the flags through the environment.
# PNG_ARM_NEON_OPT=0: the bundled libpng turns its NEON row filters on for
# aarch64, but FreeImage's Makefile.gnu does not compile them (arm/*.c), so
# libfreeimage.so had png_init_filter_functions_neon undefined.
do_build() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
  CFLAGS="-O2 -fPIC -D__ANSI__ -DDISABLE_PERF_MEASUREMENT -fvisibility=hidden -std=gnu99 -DPNG_ARM_NEON_OPT=0" \
  CXXFLAGS="-O2 -fPIC -std=gnu++14 -D__ANSI__ -DDISABLE_PERF_MEASUREMENT -fexceptions -fvisibility=hidden -Wno-ctor-dtor-privacy" \
  make -f Makefile.gnu -j"$(nproc)" LIBRARIES="-lm"; }
do_install() { install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include" && install -m644 "$B/Source/FreeImage.h" "$STAGE$PREFIX/include/" && \
  install -m755 "$B"/libfreeimage-3.18.0.so "$STAGE$PREFIX/lib/" && ln -sfn libfreeimage-3.18.0.so "$STAGE$PREFIX/lib/libfreeimage.so.3" && \
  ln -sfn libfreeimage.so.3 "$STAGE$PREFIX/lib/libfreeimage.so"; }
