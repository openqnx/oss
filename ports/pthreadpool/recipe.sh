# Pinned to the commits XNNPACK (as ported by qnx-ports) builds with.
VERSION=0-git-20250207; GIT=https://github.com/google/pthreadpool.git; COMMIT=f94ab76fe99754960035d520dce28e15b647e8cf
DEPS="fxdiv"
CMAKE_ARGS_EXTRA=( -DPTHREADPOOL_BUILD_TESTS=OFF -DPTHREADPOOL_BUILD_BENCHMARKS=OFF -DPTHREADPOOL_LIBRARY_TYPE=shared
                   -DFXDIV_SOURCE_DIR="$SRC_DIR/fxdiv" -DPTHREADPOOL_SYNC_PRIMITIVE=condvar )
# pthreadpool builds fxdiv from FXDIV_SOURCE_DIR and, with it, installs
# fxdiv.h -- the fxdiv component's header. Leave that one to fxdiv.
do_install() { DESTDIR="$STAGE" cmake --install "$B" && rm -f "$STAGE$PREFIX/include/fxdiv.h"; }
