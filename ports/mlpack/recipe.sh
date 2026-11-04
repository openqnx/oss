# mlpack: C++ machine learning library (k-means, decision trees, random
# forests, kNN, logistic/linear regression, PCA, neural networks, ...) on
# Armadillo, with its command-line programs (mlpack_kmeans, mlpack_knn,
# mlpack_linear_regression, ...). The library itself is header-only.
VERSION=4.8.0; GIT=https://github.com/mlpack/mlpack.git; REF=4.8.0
# Armadillo (on OpenBLAS) for the matrices, ensmallen for the optimisers,
# cereal for model serialisation (the CLI programs' --input_model/--output_model).
DEPS="armadillo ensmallen cereal"
BUILDSYS=cmake
# CROSSCOMPILE_EMBEDDED=OFF (patch 0001): configure like a native build
# against the installed dependencies, not mlpack's embedded-board mode.
# OpenMP off: the SDP has no libgomp. The CLI programs are built (shared,
# linking libarmadillo); Python/Julia/R/Go bindings and the test suite are not.
# STB, dr_libs (image/audio loading) stay the copies bundled in the headers.
CMAKE_ARGS_EXTRA=( -DCROSSCOMPILE_EMBEDDED=OFF -DUSE_OPENMP=OFF -DBUILD_CLI_EXECUTABLES=ON -DBUILD_SHARED_LIBS=ON
                   -DBUILD_TESTS=OFF -DBUILD_PYTHON_BINDINGS=OFF -DBUILD_JULIA_BINDINGS=OFF -DBUILD_R_BINDINGS=OFF
                   -DBUILD_GO_BINDINGS=OFF -DDOWNLOAD_DEPENDENCIES=OFF )
