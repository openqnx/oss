VERSION=2.5.3; GIT=https://github.com/uclouvain/openjpeg.git; REF=v2.5.3
DEPS="zlib libpng libtiff lcms2"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF
                   -DBUILD_CODEC=OFF -DBUILD_TESTING=OFF )
