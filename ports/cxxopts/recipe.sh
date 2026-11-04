# cxxopts: a header-only C++ command line option parser (GNU-style long and
# short options, typed values, generated --help). Header-only: the CMake build
# installs cxxopts.hpp, cxxopts.pc and the CMake package; nothing is compiled.
# candb, which bundles cxxopts 2.2, no longer installs its copy (see
# ports/candb/recipe.sh), so the two do not collide.
VERSION=3.3.1; GIT=https://github.com/jarro2783/cxxopts.git; REF=v3.3.1
BUILDSYS=cmake
# Its examples and tests are not installed: off. Unicode help text would need
# ICU: off (the upstream default).
CMAKE_ARGS_EXTRA=( -DCXXOPTS_BUILD_EXAMPLES=OFF -DCXXOPTS_BUILD_TESTS=OFF -DCXXOPTS_ENABLE_INSTALL=ON
                   -DCXXOPTS_USE_UNICODE_HELP=OFF )
