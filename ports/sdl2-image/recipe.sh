VERSION=2.8.8; GIT=https://github.com/libsdl-org/SDL_image.git; REF=release-2.8.8
SRCROOT="$SRC_DIR/sdl2-image"
DEPS="sdl2 libpng libjpeg-turbo libtiff libwebp libavif libjxl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DSDL2IMAGE_SAMPLES=OFF -DSDL2IMAGE_TESTS=OFF
                   -DSDL2IMAGE_DEPS_SHARED=OFF -DSDL2IMAGE_VENDORED=OFF
                   -DSDL2IMAGE_BACKEND_IMAGEIO=OFF )
