# 2048 (the puzzle game). A libretro core: a shared library RetroArch loads.
VERSION=0-git-20241227; GIT=https://github.com/libretro/libretro-2048.git; COMMIT=86e02d3c2dd76858db7370f5df1ccfc33b3abee1
BUILDSYS=custom
# Makefile.libretro: the plain Makefile in this repository is an NMake file
# for MSVC ("Makefile:1: *** missing separator").
do_build() { cp -a "$S/." "$B/" && make -C "$B" -f Makefile.libretro -j"$(nproc)" platform=qnx CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
  AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" fpic=-fPIC; }
do_install() { install -d "$STAGE$PREFIX/lib/libretro" && cp "$B"/*_libretro*.so "$STAGE$PREFIX/lib/libretro/"; }
