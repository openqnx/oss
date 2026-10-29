VERSION=1.51.0; GIT=https://github.com/libuv/libuv.git; REF=v1.51.0
# 1.51: the version Node.js 24 bundles and needs (uv_thread_setname is 1.50+).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DLIBUV_BUILD_TESTS=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
