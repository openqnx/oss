# CasADi: symbolic framework for nonlinear optimisation and algorithmic
# differentiation -- expression graphs (SX/MX), forward/reverse AD, NLP and QP
# solvers, integrators. Built with what it bundles: SUNDIALS (CVODES/IDAS
# integrators), CSparse, TinyXML and its own solvers (SQP method, qrqp,
# ipqp, ...); no Ipopt (not ported), no Python/MATLAB front-ends.
# WITH_DEEPBIND off: plugins are dlopen()ed with RTLD_DEEPBIND by default,
# which is glibc's and QNX does not have. WITH_SO_VERSION off: CasADi
# dlopen()s its plugins by their unversioned names (libcasadi_nlpsol_
# sqpmethod.so), so those must be the libraries themselves, in the run-time
# package, not -dev symlinks to .so.3.8.
VERSION=3.8.2; GIT=https://github.com/casadi/casadi.git; REF=3.8.2
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DWITH_DEEPBIND=OFF -DWITH_SO_VERSION=OFF -DWITH_PYTHON=OFF -DWITH_EXAMPLES=OFF )
