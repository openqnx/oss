# nanoflann: a header-only C++ library of KD-trees for nearest-neighbour and
# radius searches in point clouds (a fork of FLANN's KD-tree). Header-only:
# the CMake build installs nanoflann.hpp, nanoflann.pc and the CMake package.
VERSION=1.10.0; GIT=https://github.com/jlblancoc/nanoflann.git; REF=v1.10.0
BUILDSYS=cmake
# Its examples and its bundled-gtest unit tests are not installed: off (the
# bundled gtest also needs -lregex on QNX, which its CMake does not add).
CMAKE_ARGS_EXTRA=( -DNANOFLANN_BUILD_EXAMPLES=OFF -DNANOFLANN_BUILD_TESTS=OFF )
