VERSION=1.2.2; GIT=https://github.com/libsndfile/libsndfile.git; REF=1.2.2
DEPS="libogg libvorbis flac opus"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF
                   -DBUILD_TESTING=OFF -DENABLE_EXTERNAL_LIBS=ON -DENABLE_MPEG=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   # the build machine's Python makes the symbol files; SDP 8.0.6
                   # has a python3 for the target, which cmake found instead
                   -DPYTHON_EXECUTABLE=/usr/bin/python3 )
