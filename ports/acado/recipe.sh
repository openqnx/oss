# ACADO Toolkit: automatic control and dynamic optimisation -- optimal
# control (multiple shooting, SQP), parameter and state estimation, MPC, and
# the code generation tool that exports fast embedded C solvers for MPC/MHE
# (qpOASES-based). Autoware's MPC controller uses the exported solvers.
# The libraries (libacado_toolkit_s, libacado_casadi, qpOASES) and the CGT;
# the examples are not built: several of them run their own code generator
# at build time, which cannot run on the build host.
# patches/: -march=native is dropped when cross-compiling; the code
# generator looks for its templates in the installed tree only, and they are
# installed as data (share/acado/templates, in the run-time package).
VERSION=1.2.2_beta; GIT=https://github.com/acado/acado.git; REF=v1.2.2beta
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DACADO_WITH_EXAMPLES=OFF -DACADO_WITH_TESTING=OFF )
