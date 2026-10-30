# whisper.cpp: OpenAI Whisper speech recognition in C/C++ on ggml, CPU
# inference (the library, whisper-cli, the server), with QNX's changes
# (patches/0001, from github.com/qnx-ports/whisper.cpp; the commit QNX's
# port is based on).
VERSION=0_git20260728; GIT=https://github.com/ggml-org/whisper.cpp.git; COMMIT=97c56f1dc1d1100a9d859c865a20c82d22f823ed
DEPS=""
BUILDSYS=cmake
# It carries its own ggml (libggml*.so, ggml.h), as stable-diffusion.cpp and
# whisper.cpp each do, in versions that are not interchangeable: each gets a
# prefix of its own, /usr/local/opt/whisper.cpp, its commands linked into bin/.
# The run path is that prefix's lib/, absolute: QNX's loader does not expand
# $ORIGIN (docs/porting-hazards.md).
OPT="$PREFIX/opt/whisper.cpp"
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_PREFIX="$OPT" -DCMAKE_INSTALL_RPATH="$OPT/lib" -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DBUILD_SHARED_LIBS=ON -DGGML_STATIC=OFF -DWHISPER_BUILD_TESTS=OFF -DWHISPER_BUILD_EXAMPLES=ON
                   -DWHISPER_BUILD_SERVER=ON -DWHISPER_SDL2=OFF -DGGML_NATIVE=OFF -DGGML_OPENMP=OFF -DGGML_VULKAN=OFF -DWHISPER_CURL=OFF )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  local p; mkdir -p "$STAGE$PREFIX/bin"
  # The Silero voice-activity model the project tests with and its JFK
  # speech sample (both small): to try it, and for the test.
  install -Dm644 "$S/models/for-tests-silero-v6.2.0-ggml.bin" "$STAGE$OPT/share/models/ggml-silero-v6.2.0.bin" || return 1
  install -Dm644 "$S/samples/jfk.wav" "$STAGE$OPT/share/samples/jfk.wav" || return 1
  for p in whisper-cli whisper-server; do [ -e "$STAGE$OPT/bin/$p" ] && ln -sf "../opt/whisper.cpp/bin/$p" "$STAGE$PREFIX/bin/$p"; done; return 0
}
