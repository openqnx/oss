# sophus: C++ Lie groups for 2D and 3D geometry on Eigen -- SO(2), SO(3),
# SE(2), SE(3), RxSO(2/3), Sim(2/3) with exp/log, group actions, Jacobians,
# interpolation and averaging. Header-only: the port installs the headers in
# include/sophus/ and the CMake package (Sophus::Sophus, which pulls in
# Eigen3::Eigen) in share/sophus/cmake.
#
# Nothing had to change for QNX. Upstream's test programs are not built
# (BUILD_SOPHUS_TESTS=OFF): they would need Ceres for part of them and
# cannot run on the build machine anyway; tests/src/test_sophus.cpp checks
# the group operations on the target instead. 1.24.6 formats its messages
# with iostreams, so there is no dependency on fmt as in the 1.22 series.
VERSION=1.24.6; GIT=https://github.com/strasdat/Sophus.git; REF=1.24.6
DEPS="eigen"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SOPHUS_TESTS=OFF -DBUILD_PYTHON_BINDINGS=OFF -DSOPHUS_INSTALL=ON )
