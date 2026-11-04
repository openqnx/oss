# ensmallen: header-only C++ library of numerical optimisers (L-BFGS, SGD
# variants, Adam, CMA-ES, simulated annealing, ...) on Armadillo matrices;
# mlpack's optimisation layer. Header-only: installs include/ensmallen* and
# the CMake package (ensmallen::ensmallen, which pulls in Armadillo).
VERSION=3.11.1; GIT=https://github.com/mlpack/ensmallen.git; REF=3.11.1
# Armadillo is required at configure time (find_package) and by consumers.
DEPS="armadillo"
# Header-only, so the package has no NEEDED entries to derive dependencies
# from: name what a consumer compiles and links against.
RUNTIME_DEPS="armadillo-dev"
BUILDSYS=cmake
# OpenMP off: the SDP has no libgomp, and with USE_OPENMP on ensmallen
# defines ENS_USE_OPENMP for consumers even when OpenMP is not found.
# Bandicoot (GPU matrices) is not ported: off. Tests are not installed.
CMAKE_ARGS_EXTRA=( -DUSE_OPENMP=OFF -DUSE_BANDICOOT=OFF -DBUILD_TESTS=OFF )
