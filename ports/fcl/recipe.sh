VERSION=0.7.0; GIT=https://github.com/flexible-collision-library/fcl.git; REF=0.7.0
DEPS="eigen libccd octomap"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DFCL_WITH_OCTOMAP=ON
                   -DFCL_USE_X64_SSE=OFF -DFCL_STATIC_LIBRARY=OFF )
