# Header-only half-precision conversions (used by XNNPACK). No releases.
VERSION=0-git-20260620; GIT=https://github.com/Maratyszcza/FP16.git; COMMIT=782eea126dc5c755827be751a099eb01826175cf
CMAKE_ARGS_EXTRA=( -DFP16_BUILD_TESTS=OFF -DFP16_BUILD_BENCHMARKS=OFF )
