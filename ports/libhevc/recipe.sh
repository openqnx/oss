# libhevc: Android's HEVC (H.265) video decoder and encoder libraries (from
# Ittiam, the code behind Android's software HEVC codecs), with the hevcdec
# and hevcenc example programs.
VERSION=1.8.5; GIT=https://github.com/ittiam-systems/libhevc.git; REF=v1.8.5
BUILDSYS=cmake
# SYSTEM_NAME/SYSTEM_PROCESSOR pick the SIMD code; they default to the build
# machine's. aarch64: the NEON assembly and intrinsics; x86_64: the SSSE3/SSE4.2
# intrinsics (the project builds everything with -msse4.2 -- any x86_64 board
# QNX 8 runs on has it). Tests and benchmarks would download googletest and
# google-benchmark at configure time: off. PIC so that the static libraries
# can be linked into shared objects.
CMAKE_ARGS_EXTRA=( -DENABLE_TESTS=OFF -DENABLE_BENCHMARKS=OFF -DSYSTEM_NAME=QNX
                   -DSYSTEM_PROCESSOR=$( [ "$QNX_ARCH" = aarch64le ] && echo aarch64 || echo x86_64 )
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
# Upstream (like Android) builds static libraries and installs nothing:
# install the two libraries, their public API headers (in include/libhevc)
# and the two example programs.
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/libhevc" "$STAGE$PREFIX/bin" &&
  install -m 644 "$B/libhevcdec.a" "$B/libhevcenc.a" "$STAGE$PREFIX/lib/" &&
  install -m 644 "$S/common/ihevc_typedefs.h" "$S/common/iv.h" "$S/common/ivd.h" \
                 "$S/decoder/ihevcd_cxa.h" "$S/encoder/ihevce_api.h" "$S/encoder/ihevce_plugin.h" \
                 "$S/encoder/itt_video_api.h" "$STAGE$PREFIX/include/libhevc/" &&
  install -m 755 "$B/hevcdec" "$B/hevcenc" "$STAGE$PREFIX/bin/"
}
