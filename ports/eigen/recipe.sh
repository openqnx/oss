VERSION=3.4.0; GIT=https://gitlab.com/libeigen/eigen.git; REF=3.4.0
# EIGEN_TEST_CXX11=ON only changes which standard the configure-time probes use.
# Left OFF, Eigen appends -std=c++03 to CMAKE_CXX_FLAGS, and QNX 8.0's libc++
# refuses to compile below C++11 -- so FindStandardMathLibrary can never succeed
# and configure dies with "Can't link to the standard math library".
CMAKE_ARGS_EXTRA=( -DEIGEN_BUILD_DOC=OFF -DEIGEN_BUILD_TESTING=OFF
                   -DEIGEN_BUILD_PKGCONFIG=ON -DEIGEN_TEST_CXX11=ON )
