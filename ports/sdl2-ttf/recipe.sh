VERSION=2.24.0; GIT=https://github.com/libsdl-org/SDL_ttf.git; REF=release-2.24.0
SRCROOT="$SRC_DIR/sdl2-ttf"
DEPS="sdl2 freetype harfbuzz"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DSDL2TTF_SAMPLES=OFF -DSDL2TTF_VENDORED=OFF
                   -DSDL2TTF_HARFBUZZ=ON )
