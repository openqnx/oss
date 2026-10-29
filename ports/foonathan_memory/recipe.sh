VERSION=0.7-3; GIT=https://github.com/foonathan/memory.git; REF=v0.7-3
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DFOONATHAN_MEMORY_BUILD_EXAMPLES=OFF
                   -DFOONATHAN_MEMORY_BUILD_TESTS=OFF
                   # nodesize_dbg links -static; QNX ships no libsocket.a
                   -DFOONATHAN_MEMORY_BUILD_TOOLS=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
