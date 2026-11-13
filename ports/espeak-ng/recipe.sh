# espeak-ng: eSpeak NG, a compact formant speech synthesizer for more than a
# hundred languages: libespeak-ng (text to phonemes, text to PCM samples), the
# espeak-ng program and the compiled voice data (phoneme tables, one
# dictionary per language, voice and language definitions).
#
# What is different on QNX, and why:
# - No audio device output. pcaudiolib (ALSA/PulseAudio/OSS back ends) is not
#   ported, so the program writes WAV files (-w, --stdout) and the library
#   hands the samples to the application's callback (AUDIO_OUTPUT_RETRIEVAL,
#   AUDIO_OUTPUT_SYNCHRONOUS). Playing them is the application's business.
# - The voice data is compiled by espeak-ng itself during the build. The QNX
#   program cannot run on the build machine, so a native espeak-ng is built
#   first from the same source (build/<arch>/host/espeak-ng, static, only the
#   program) and compiles the data (patches/0002, ESPEAK_NG_HOST_BIN). The
#   compiled files are little endian, like both QNX targets.
# - The data directory is found relative to the library when the configured
#   /usr/local/share/espeak-ng-data does not exist (patches/0003): packages
#   are also installed under /data/oss. ESPEAK_DATA_PATH still overrides.
# - sonic (faster speech rates, Apache-2.0) is not a component here; upstream
#   fetches it with git while configuring. The same commit is downloaded as
#   a checksummed archive instead and compiled into the library, as upstream
#   does when no libsonic is installed.
# - MBROLA voices are left out (the mbrola program is not ported).
VERSION=1.52.0; GIT=https://github.com/espeak-ng/espeak-ng.git; REF=1.52.0
DEPS=""
BUILDSYS=cmake
# The commit cmake/deps.cmake names for sonic.
SONIC_COMMIT=fbf75c3d6d846bad3bb3d456cbc5d07d9fd8c104
SONIC_SHA256=715827b5a39b79e56e44397d7b845910df996d4cca74777b3b61629b1ddc98c1
SONIC_DIR="$SRC_DIR/sonic-espeak-ng-$SONIC_COMMIT"
# PTHREAD_LIB=c: upstream looks for a libpthread to decide whether the
# asynchronous API (espeak_Synth() returning at once, events, the command
# queue) can be built. QNX has no libpthread, the threads are in libc.
# USE_LIBPCAUDIO=OFF, USE_MBROLA=OFF: see above; stated so that nothing found
# in staging or on the build machine changes the result.
ESPEAK_ARGS=( -DFETCHCONTENT_SOURCE_DIR_SONIC-GIT="$SONIC_DIR" -DFETCHCONTENT_FULLY_DISCONNECTED=ON
              -DUSE_LIBSONIC=ON -DUSE_LIBPCAUDIO=OFF -DUSE_MBROLA=OFF -DUSE_KLATT=ON -DUSE_SPEECHPLAYER=ON
              -DCMAKE_DISABLE_FIND_PACKAGE_Git=ON )
do_configure() {
  if [ ! -f "$SONIC_DIR/sonic.c" ]; then
    local tb="$DIST_DIR/sonic-$SONIC_COMMIT.tar.gz"
    [ -f "$tb" ] || curl -fsSL --retry 3 -o "$tb" "https://github.com/waywardgeek/sonic/archive/$SONIC_COMMIT.tar.gz" || return 1
    echo "$SONIC_SHA256  $tb" | sha256sum -c - || return 1
    rm -rf "$SONIC_DIR"; mkdir -p "$SONIC_DIR" && tar xf "$tb" -C "$SONIC_DIR" --strip-components=1 || return 1
  fi
  # The native espeak-ng that compiles the data: the build machine's own
  # compiler, none of the cross environment.
  local HB="$BUILD_DIR/$QNX_ARCH/host/espeak-ng"
  if [ ! -x "$HB/src/espeak-ng" ]; then
    rm -rf "$HB"
    ( unset CC CXX CPP AR LD CFLAGS CXXFLAGS CPPFLAGS LDFLAGS PKG_CONFIG PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR PKG_CONFIG_PATH
      cmake -S "$S" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
        -DBUILD_SHARED_LIBS=OFF -DUSE_ASYNC=OFF "${ESPEAK_ARGS[@]}" \
      && cmake --build "$HB" -j4 --target espeak-ng-bin ) || { echo "native espeak-ng build failed"; return 1; }
  fi
  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
    -DBUILD_SHARED_LIBS=ON -DPTHREAD_LIB=c -DUSE_ASYNC=ON \
    -DESPEAK_NG_HOST_BIN="$HB/src/espeak-ng" "${ESPEAK_ARGS[@]}"
}
# -j4: other builds share the machine.
do_build() { cmake --build "$B" -j4; }
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  # The licenses of the parts: GPL-3.0-or-later (espeak-ng), BSD-2-Clause
  # (ieee80.c), Apache-2.0 (sonic, getopt), Unicode (ucd-tools data).
  install -d "$STAGE$PREFIX/share/licenses/espeak-ng" || return 1
  install -m644 "$S"/COPYING "$S"/COPYING.APACHE "$S"/COPYING.BSD2 "$S"/COPYING.UCD "$STAGE$PREFIX/share/licenses/espeak-ng/"
}
