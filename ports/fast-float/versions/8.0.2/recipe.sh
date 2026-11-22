# fast_float: header-only C++ implementation of from_chars for floating-point
# and integer types, several times faster than strtod (Eisel-Lemire
# algorithm). Build dependency of folly.
VERSION=8.0.2; GIT=https://github.com/fastfloat/fast_float.git; REF=v8.0.2
CMAKE_ARGS_EXTRA=( -DFASTFLOAT_TEST=OFF -DFASTFLOAT_INSTALL=ON )
