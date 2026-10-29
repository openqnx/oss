# Mr.Boom (8-player Bomberman clone). A libretro core: a shared library RetroArch loads.
VERSION=0-git-20260625; GIT=https://github.com/libretro/mrboom-libretro.git; COMMIT=40ac32020b540c4bb418c7d798a990932a65da04
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" -j"$(nproc)" platform=qnx CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
  AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" fpic=-fPIC; }
do_install() { install -d "$STAGE$PREFIX/lib/libretro" && cp "$B"/*_libretro*.so "$STAGE$PREFIX/lib/libretro/"; }
