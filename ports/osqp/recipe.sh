# OSQP 0.6: the Operator Splitting QP solver -- convex quadratic programs
# (min 1/2 x'Px + q'x subject to l <= Ax <= u) with sparse matrices, by ADMM,
# with the bundled QDLDL direct linear solver (a git submodule). Version 0.6.3
# is the one QNX ships and the 0.6 API (osqp_setup(&work, data, settings))
# that osqp-eigen and Autoware's MPC use; OSQP 1.x changed that API.
# The C library (shared and static); its unit tests and MKL Pardiso
# interface are not built.
VERSION=0.6.3; GIT=https://github.com/osqp/osqp.git; REF=v0.6.3
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DUNITTESTS=OFF -DENABLE_MKL_PARDISO=OFF )
