# sdl2-compat: the SDL2 API implemented on top of SDL3 (the image's SDL2).
# Installs libSDL2-2.0 like the sdl2 component, so use one or the other.
VERSION=2.32.60; GIT=https://github.com/libsdl-org/sdl2-compat.git; REF=release-2.32.60
DEPS="sdl3"
CONFLICTS="sdl2"
CMAKE_ARGS_EXTRA=( -DSDL2COMPAT_TESTS=OFF -DSDL2COMPAT_INSTALL_TESTS=OFF )
