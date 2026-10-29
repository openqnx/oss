VERSION=stable; GIT=https://chromium.googlesource.com/libyuv/libyuv; REF=stable
DEPS="libjpeg-turbo"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
