VERSION=2.1.12; GIT=https://github.com/libevent/libevent.git; REF=release-2.1.12-stable
DEPS="openssl"
CMAKE_ARGS_EXTRA=( -DEVENT__LIBRARY_TYPE=SHARED -DEVENT__DISABLE_BENCHMARK=ON
                   -DEVENT__DISABLE_TESTS=ON -DEVENT__DISABLE_SAMPLES=ON
                   -DEVENT__DISABLE_REGRESS=ON )
