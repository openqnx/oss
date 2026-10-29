# Header-only fixed-point division (used by pthreadpool, XNNPACK). No releases;
# pinned to the commit XNNPACK uses.
VERSION=0-git-20200417; GIT=https://github.com/Maratyszcza/FXdiv.git; COMMIT=b408327ac2a15ec3e43352421954f5b1967701d1
CMAKE_ARGS_EXTRA=( -DFXDIV_BUILD_TESTS=OFF -DFXDIV_BUILD_BENCHMARKS=OFF )
