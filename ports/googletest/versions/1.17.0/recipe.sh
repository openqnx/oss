VERSION=1.17.0; GIT=https://github.com/google/googletest.git; REF=v1.17.0
CMAKE_ARGS_EXTRA=( -Dgtest_force_shared_crt=ON -DBUILD_SHARED_LIBS=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
