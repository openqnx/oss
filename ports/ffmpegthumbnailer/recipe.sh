VERSION=2.2.3; GIT=https://github.com/dirkvdb/ffmpegthumbnailer.git; REF=2.2.3
DEPS="ffmpeg libpng libjpeg-turbo"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_GIO=OFF -DENABLE_THUMBNAILER=OFF
                   -DENABLE_TESTS=OFF )
