VERSION=2.4.2; GIT=https://github.com/dirkvdb/ffmpegthumbnailer.git; REF=v2.4.2
DEPS="ffmpeg libpng libjpeg-turbo"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_GIO=OFF -DENABLE_THUMBNAILER=OFF
                   -DENABLE_TESTS=OFF )
