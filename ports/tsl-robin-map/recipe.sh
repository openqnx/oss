# tsl-robin-map: tsl::robin_map and tsl::robin_set, C++ hash map and hash set
# with open addressing and robin hood (backward shift deletion) hashing --
# drop-in alternatives to std::unordered_map/set that keep their entries in
# one contiguous array. Header-only: include/tsl/ and the CMake package
# tsl-robin-map (target tsl::robin_map) in share/cmake/tsl-robin-map.
# kiss-icp's voxel hash map is built on it.
#
# Nothing had to change for QNX; upstream's tests (Boost.Test) are a separate
# CMake project and are not built. tests/src/test_tsl_robin_map.cpp checks
# the containers on the target.
VERSION=1.4.1; GIT=https://github.com/Tessil/robin-map.git; REF=v1.4.1
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DTSL_ROBIN_MAP_ENABLE_INSTALL=ON )
