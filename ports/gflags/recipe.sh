VERSION=2.2.2; GIT=https://github.com/gflags/gflags.git; REF=v2.2.2
# REGISTER_*: gflags installs a CMake user-registry entry into $HOME/.cmake
# (packaged as home/<user>/.cmake/packages/gflags/...); not on a target.
CMAKE_ARGS_EXTRA=( -DREGISTER_INSTALL_PREFIX=OFF -DREGISTER_BUILD_DIR=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_gflags_LIB=ON
                   -DBUILD_TESTING=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
