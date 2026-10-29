VERSION=7.11.0; GIT=https://github.com/DrTimothyAldenDavis/SuiteSparse.git; REF=v7.11.0
DEPS="openblas"
CMAKE_ARGS_EXTRA=( -DSUITESPARSE_USE_FORTRAN=OFF -DSUITESPARSE_USE_CUDA=OFF -DSUITESPARSE_USE_OPENMP=OFF
                   -DBLA_VENDOR=OpenBLAS -DBLAS_LIBRARIES="$STAGE$PREFIX/lib/libopenblas.so" -DLAPACK_LIBRARIES="$STAGE$PREFIX/lib/libopenblas.so"
                   -DSUITESPARSE_DEMOS=OFF -DBUILD_TESTING=OFF -DSUITESPARSE_ENABLE_PROJECTS="suitesparse_config;amd;btf;camd;ccolamd;colamd;cholmod;cxsparse;klu;ldl;umfpack;rbio;spqr" )
