# libretro sample cores (test patterns, input/audio tests). A libretro core: a shared library RetroArch loads.
VERSION=0-git-20221110; GIT=https://github.com/libretro/libretro-samples.git; COMMIT=bce193bc1b8c9a3da43b2ead0158a69e28b37ed8
BUILDSYS=custom
# The repository is a set of separate sample cores without a top-level
# Makefile ("No targets specified"); build the test core (tests/test), which
# exercises RetroArch's video, audio and input paths without game content.
# -D_QNX_SOURCE: its Makefile compiles with -std=c99, which on QNX hides M_PI.
do_build() { cp -a "$S/." "$B/" && make -C "$B/tests/test" -j"$(nproc)" platform=qnx CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx" \
  AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" fpic=-fPIC; }
do_install() { install -d "$STAGE$PREFIX/lib/libretro" && cp "$B"/tests/test/*_libretro*.so "$STAGE$PREFIX/lib/libretro/"; }
