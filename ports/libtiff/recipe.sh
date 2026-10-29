VERSION=4.7.0; GIT=https://gitlab.com/libtiff/libtiff.git; REF=v4.7.0
DEPS="zlib xz zstd libjpeg-turbo libwebp"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -Dtiff-tools=OFF -Dtiff-tests=OFF
                   -Dtiff-docs=OFF -Dtiff-contrib=OFF -Dlerc=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
