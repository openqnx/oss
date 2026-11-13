# piper: Piper, a fast local neural text to speech system (VITS voices run
# with ONNX Runtime, text turned into phonemes by espeak-ng). This is the
# maintained upstream, OHF-Voice/piper1-gpl (GPL-3.0-or-later): rhasspy/piper
# and rhasspy/piper-phonemize, the earlier C++ program and its phonemizer
# library, are archived, and their successor is no longer split in two --
# libpiper does the phonemizing itself, with espeak-ng's own API. Built from
# its libpiper/ directory: libpiper.so with the C API of piper.h
# (piper_create(), piper_synthesize_start()/_next(): text in, float samples
# out) and piper_exe, the command line program (text on stdin, WAV files or
# raw samples out). The Python package (training, HTTP server) is not built.
#
# A voice is two files, <voice>.onnx and <voice>.onnx.json; none is part of
# this component (ports/piper-voice-en-us-joe is one).
#
# What is different here, and why:
# - espeak-ng is the espeak-ng component, not a private static copy that the
#   build downloads and compiles (patches/0001, PIPER_USE_SYSTEM_ESPEAK_NG):
#   one library and one share/espeak-ng-data on the target. piper_exe then
#   needs no --espeak_data; upstream finds the data through /proc/self/exe,
#   which QNX does not have. libpiper needs
#   espeak_TextToPhonemesWithTerminator(), newer than espeak-ng 1.52.0:
#   ports/espeak-ng backports it.
# - ONNX Runtime is the onnxruntime component (1.20.1) through its CMake
#   package, not the 1.22 binary release upstream downloads -- there is none
#   for QNX. One call that 1.20 does not have is replaced (patches/0002).
# - patches/0003: QNX's major()/minor() macros against a bundled header.
VERSION=1.8.0; GIT=https://github.com/OHF-Voice/piper1-gpl.git; REF=v1.8.0
SUBDIR=libpiper
DEPS="espeak-ng onnxruntime"
BUILDSYS=cmake
# onnxruntime_DIR: find_package(onnxruntime) must find the staged package;
# if it did not, the build would download a Linux binary.
CMAKE_ARGS_EXTRA=( -DPIPER_USE_SYSTEM_ESPEAK_NG=ON -DPIPER_BUILD_TESTS=OFF
                   -Donnxruntime_DIR="$STAGE$PREFIX/lib/cmake/onnxruntime" )
# -j4: other builds share the machine.
do_build() { cmake --build "$B" -j4; }
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  install -Dm644 "$SRCROOT/COPYING" "$STAGE$PREFIX/share/licenses/piper/COPYING"
}
