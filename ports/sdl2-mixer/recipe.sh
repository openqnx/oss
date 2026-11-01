# SDL_mixer 2: multi-channel audio mixing and music playback on SDL2 (needed
# by pygame.mixer). Decoders: WAV/AIFF/VOC (built in), FLAC (dr_flac), MP3
# (minimp3), Ogg Vorbis (stb_vorbis), MIDI (Timidity) -- all bundled with
# SDL_mixer, no library needed -- and Opus through the ported opusfile,
# linked rather than dlopened (SDL2MIXER_DEPS_SHARED=OFF: what is loaded at
# run time is invisible to the package's dependencies). MOD (libxmp),
# WavPack and game-music-emu are off: not ported here.
VERSION=2.8.2; GIT=https://github.com/libsdl-org/SDL_mixer.git; REF=release-2.8.2
SRCROOT="$SRC_DIR/sdl2-mixer"
DEPS="sdl2 opusfile"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DSDL2MIXER_SAMPLES=OFF -DSDL2MIXER_VENDORED=OFF
                   -DSDL2MIXER_DEPS_SHARED=OFF
                   -DSDL2MIXER_FLAC=ON -DSDL2MIXER_FLAC_DRFLAC=ON -DSDL2MIXER_FLAC_LIBFLAC=OFF
                   -DSDL2MIXER_MP3=ON -DSDL2MIXER_MP3_MINIMP3=ON -DSDL2MIXER_MP3_MPG123=OFF
                   -DSDL2MIXER_VORBIS=STB -DSDL2MIXER_OPUS=ON
                   -DSDL2MIXER_MOD=OFF -DSDL2MIXER_GME=OFF -DSDL2MIXER_WAVPACK=OFF
                   -DSDL2MIXER_MIDI=ON -DSDL2MIXER_MIDI_TIMIDITY=ON -DSDL2MIXER_MIDI_FLUIDSYNTH=OFF )
# Against the sdl2 component's own headers and CMake package (its package
# root), not whatever is in staging: sdl2-compat installs the same files,
# with a newer version number, over sdl2's there, and a program built against
# them refuses to run on the sdl2 it is packaged with ("SDL downgrade").
CMAKE_ARGS_EXTRA+=( -DSDL2_DIR="$WS/pkgroot/$QNX_ARCH/sdl2$PREFIX/lib/cmake/SDL2" )
