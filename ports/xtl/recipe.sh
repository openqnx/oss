# xtl: QuantStack's header-only C++ template library (basic tools, containers,
# algorithms) used by xtensor and xeus. Header-only: the CMake build installs
# the headers and the xtlConfig.cmake package files; nothing is compiled.
VERSION=0.8.2; GIT=https://github.com/xtensor-stack/xtl.git; REF=0.8.2
BUILDSYS=cmake
# The nlohmann_json integration header (xtl/xjson.hpp) is always installed;
# it needs nlohmann_json only in the consumer, so no DEPS. Tests are off.
CMAKE_ARGS_EXTRA=( -DBUILD_TESTS=OFF -DDOWNLOAD_GTEST=OFF )
