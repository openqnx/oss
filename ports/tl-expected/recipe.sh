# tl-expected: TartanLlama's header-only C++11/14/17 implementation of
# std::expected (a value or an error) with the monadic and_then/map/or_else
# extensions. Header-only: installs tl/expected.hpp and the CMake package
# (tl::expected), nothing is compiled.
VERSION=1.3.1; GIT=https://github.com/TartanLlama/expected.git; REF=v1.3.1
BUILDSYS=cmake
# Its Catch2 tests are fetched from the network at configure time, and the
# CPack (.deb/.tar) packaging is not used here: both off.
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DEXPECTED_BUILD_TESTS=OFF -DEXPECTED_BUILD_PACKAGE=OFF )
