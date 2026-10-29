VERSION=2.2.0; GIT=https://github.com/ceres-solver/ceres-solver.git; REF=2.2.0
DEPS="eigen glog gflags"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF
                   -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARKS=OFF
                   -DMINIGLOG=OFF -DGFLAGS=ON -DSUITESPARSE=OFF
                   -DLAPACK=OFF -DCUDA=OFF -DUSE_CUDA=OFF
                   -DEigen3_DIR="$STAGE$PREFIX/share/eigen3/cmake"
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
