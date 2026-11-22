VERSION=3.4.0; GIT=https://github.com/google/double-conversion.git; REF=v3.4.0
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON )
# double-conversion 3.4.0 needs a newer Meson/CMake than the build image's default
HOSTTOOLS_NEW=1
