# fake-08 (PICO-8 emulator). A libretro core: a shared library RetroArch loads.
VERSION=0-git-20260613; GIT=https://github.com/jtothebell/fake-08.git; COMMIT=814991a2571ad3970e386cef48f3b148aa1c27b9
BUILDSYS=custom
# The libretro core is platform/libretro (the top-level Makefile builds the
# console ports: 3ds, switch, wiiu...), as qnx-ports builds it.
do_build() { cp -a "$S/." "$B/" && make -C "$B/platform/libretro" -j"$(nproc)" platform=qnx CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
  AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" fpic=-fPIC; }
do_install() { install -d "$STAGE$PREFIX/lib/libretro" && cp "$B"/platform/libretro/*_libretro*.so "$STAGE$PREFIX/lib/libretro/"; }
