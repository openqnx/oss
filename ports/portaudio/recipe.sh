# portaudio: portable real-time audio input/output C library (python3-
# sounddevice, Audacity-like programs, many SDR and speech tools use it).
VERSION=19.7.0; GIT=https://github.com/PortAudio/portaudio.git; REF=v19.7.0
DEPS=""
BUILDSYS=cmake
# The ALSA host API, on QNX 8's ALSA-compatible libasound (the SDP's, which
# talks to io-snd); linked directly, not dlopen()ed. No JACK (not ported).
# Shared library only.
CMAKE_ARGS_EXTRA=( -DPA_USE_ALSA=ON -DPA_ALSA_DYNAMIC=OFF -DPA_USE_JACK=OFF
                   -DPA_BUILD_SHARED=ON -DPA_BUILD_STATIC=OFF -DPA_BUILD_TESTS=OFF -DPA_BUILD_EXAMPLES=OFF )
