# raylib: a simple C library for videogames programming (2D/3D drawing,
# textures, text, models, audio, math).
# Platform layer: SDL2 (PLATFORM=SDL) with raylib's software renderer
# (rlsw, OPENGL_VERSION=Software). QNX has no OpenGL for Wayland clients --
# the SDP's EGL/GLES drive QNX Screen -- so raylib's GLFW and DRM platforms
# could open no context; on SDL the software renderer draws into the SDL
# window surface, which works on SDL2's Wayland backend (shared memory) and
# on its dummy driver (headless). SDL3 is not used: here it is X11-only
# (find_package would prefer it).
VERSION=6.2; GIT=https://github.com/raysan5/raylib.git; REF=6.2
DEPS="sdl2"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=OFF
                   -DPLATFORM=SDL -DOPENGL_VERSION=Software
                   -DCMAKE_DISABLE_FIND_PACKAGE_SDL3=ON )
# Against the sdl2 component's own headers and CMake package (its package
# root), not whatever is in staging: sdl2-compat installs the same files,
# with a newer version number, over sdl2's there, and a program built against
# them refuses to run on the sdl2 it is packaged with ("SDL downgrade").
CMAKE_ARGS_EXTRA+=( -DSDL2_DIR="$WS/pkgroot/$QNX_ARCH/sdl2$PREFIX/lib/cmake/SDL2" )
