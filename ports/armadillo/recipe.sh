# armadillo: C++ linear algebra library (matrices, decompositions, solvers,
# statistics) with a MATLAB-like API, on top of BLAS and LAPACK.
VERSION=15.6.2
TARBALL=https://downloads.sourceforge.net/arma/armadillo-15.6.2.tar.xz
SHA256=63ee9351b54994dff7b4e2094a86ace48cfb41354cb65893a9dab7c8a16382c4
# BLAS and LAPACK both come from OpenBLAS, which is built with its LAPACK
# (C_LAPACK): OPENBLAS_PROVIDES_LAPACK makes the libarmadillo wrapper link
# libopenblas only. Without it CMake also picked up clapack's reference
# liblapack/libblas from staging, whose libf2c needs a Fortran MAIN__.
# ARPACK and SuperLU (sparse eigen/solvers) are not ported: not used.
DEPS="openblas"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DOPENBLAS_PROVIDES_LAPACK=ON -DALLOW_FLEXIBLAS_LINUX=OFF -DBUILD_SMOKE_TEST=OFF )
