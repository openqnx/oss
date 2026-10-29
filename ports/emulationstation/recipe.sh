# EmulationStation, RetroPie's game browser, at the upstream commit qnx-ports'
# port is based on (patches/: QNX support). Uses GLES2 (from the QNX SDP) and
# SDL2. Part of the RetroPie stack.
VERSION=2.11-git-20241212; GIT=https://github.com/RetroPie/EmulationStation.git; COMMIT=51b6f4162d8e89be2ef7a2c9d7c232ac5b31022b
DEPS="sdl2 freeimage freetype curl pugixml rapidjson nanosvg vlc"
CMAKE_ARGS_EXTRA=( -DGLES=ON -DCMAKE_NO_SYSTEM_FROM_IMPORTED=ON -DCPU="$QNX_CPU"
                   -DSDL2_LIBRARY="$STAGE$PREFIX/lib/libSDL2.so" -DSDL2_INCLUDE_DIR="$STAGE$PREFIX/include/SDL2"
                   -DSDL2MAIN_LIBRARY="$STAGE$PREFIX/lib/libSDL2main.a" -DSDL2_NO_DEFAULT_PATH=ON
                   -DFreeImage_LIBRARY="$STAGE$PREFIX/lib/libfreeimage.so" -DFreeImage_INCLUDE_DIR="$STAGE$PREFIX/include" )
