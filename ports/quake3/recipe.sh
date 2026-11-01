# ioquake3: the maintained open-source Quake III Arena engine -- the
# dedicated server (ioq3ded), the client engine (ioquake3, on SDL2, with
# the GL1/GL2 renderers it loads at run time) and the native game modules
# (qagame, cgame, ui for baseq3 and missionpack). No game data: the
# retail/demo pak files are not free; the engine runs any pk3 it is given.
# ioquake3 makes no releases any more ("rolling" builds of master): a
# pinned commit of master.
VERSION=1.36-git-20261012; GIT=https://github.com/ioquake/ioq3.git; COMMIT=87a99d18206e878a20289f05ac3c16005dcae5c3
DEPS="sdl2"
CMAKE_ARGS_EXTRA=( -DBUILD_SERVER=ON -DBUILD_CLIENT=ON -DBUILD_GAME_LIBRARIES=ON
                   # QVM bytecode needs q3lcc/q3asm run on the build machine:
                   # the native modules do the same job.
                   -DBUILD_GAME_QVMS=OFF
                   -DUSE_RENDERER_DLOPEN=ON
                   # No OpenAL port (SDL audio is used), no HTTP downloads,
                   # no Mumble link (a Linux/Windows shared-memory protocol).
                   -DUSE_OPENAL=OFF -DUSE_HTTP=OFF -DUSE_MUMBLE=OFF
                   # zlib, libjpeg, ogg/vorbis/opus: the engine's bundled copies.
                   -DUSE_INTERNAL_LIBS=ON -DUSE_INTERNAL_SDL=OFF
                   # The engine's own layout (binaries, renderers and the
                   # baseq3/missionpack module directories side by side),
                   # in a prefix of its own as on Linux (/opt/quake3); the
                   # game data goes into its baseq3/. It looks for both
                   # next to its binary -- the directory argv[0] names, so
                   # not when started through the bin/ links -- and then in
                   # DEFAULT_BASEDIR, which is the install directory (patches/0003).
                   -DCMAKE_INSTALL_PREFIX="$PREFIX/opt/quake3"
                   -DDEFAULT_BASEDIR="$PREFIX/opt/quake3" )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  mkdir -p "$STAGE$PREFIX/bin"
  ln -sf ../opt/quake3/ioq3ded "$STAGE$PREFIX/bin/ioq3ded"
  ln -sf ../opt/quake3/ioquake3 "$STAGE$PREFIX/bin/ioquake3"
}
