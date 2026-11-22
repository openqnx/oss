# fast_float: header-only C++ implementation of from_chars for floating-point
# and integer types, several times faster than strtod (Eisel-Lemire
# algorithm). Build dependency of folly.
VERSION=8.3.3; GIT=https://github.com/fastfloat/fast_float.git; REF=v8.3.3
CMAKE_ARGS_EXTRA=( -DFASTFLOAT_TEST=OFF -DFASTFLOAT_INSTALL=ON )
