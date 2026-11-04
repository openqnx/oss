# doctest: a fast, single-header C++ unit testing framework (TEST_CASE,
# SUBCASE, CHECK, templated test cases). Header-only: the CMake build installs
# doctest/doctest.h (plus its parts and extensions) and the CMake package.
VERSION=2.5.3; GIT=https://github.com/doctest/doctest.git; REF=v2.5.3
BUILDSYS=cmake
# Its own tests and examples are not installed: off. Patch 0001 makes the
# header recognise QNX's libc++ under GCC (see the patch).
CMAKE_ARGS_EXTRA=( -DDOCTEST_WITH_TESTS=OFF -DDOCTEST_WITH_MAIN_IN_STATIC_LIB=OFF )
