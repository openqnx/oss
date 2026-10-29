# retro8 (PICO-8 virtual console). A libretro core: a shared library RetroArch loads.
VERSION=0-git-20260912; GIT=https://github.com/Jakz/retro8.git; COMMIT=ecee6594c5b953966ee0515f3edac4032c7ed40f
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" -j"$(nproc)" platform=qnx CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
  AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" fpic=-fPIC; }
do_install() { install -d "$STAGE$PREFIX/lib/libretro" && cp "$B"/*_libretro*.so "$STAGE$PREFIX/lib/libretro/"; }
