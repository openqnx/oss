# pinocchio: rigid body dynamics for poly-articulated systems (forward
# kinematics, Jacobians, RNEA, CRBA, ABA and their analytical derivatives) on
# Eigen, with a URDF parser built on urdfdom.
#
# Built: the C++ libraries (pinocchio_default, pinocchio_parsers and the
# pinocchio umbrella target) and the headers.
# Left out: the Python bindings (need eigenpy and a target Python build of
# Boost.Python), collision support (BUILD_WITH_COLLISION_SUPPORT needs
# coal/hpp-fcl, which is not ported here and would be a port of its own),
# SDF, CasADi/CppAD/autodiff, OpenMP, examples, utils, tests and benchmarks.
VERSION=3.9.0; GIT=https://github.com/stack-of-tasks/pinocchio.git; REF=v3.9.0
DEPS="eigen boost urdfdom urdfdom_headers console_bridge tinyxml2"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF
                   -DBUILD_PYTHON_INTERFACE=OFF -DBUILD_WITH_LIBPYTHON=OFF
                   -DBUILD_EXAMPLES=OFF -DBUILD_UTILS=OFF -DBUILD_BENCHMARK=OFF
                   -DBUILD_WITH_URDF_SUPPORT=ON -DBUILD_WITH_SDF_SUPPORT=OFF
                   -DBUILD_WITH_COLLISION_SUPPORT=OFF
                   -DBUILD_WITH_OPENMP_SUPPORT=OFF
                   -DINSTALL_DOCUMENTATION=OFF -DBUILD_DOCUMENTATION=OFF )
# The explicit template instantiations of the main library need 2-3 GB of
# memory per compiler process: two jobs, not one per core.
do_build() { cmake --build "$B" -j2; }
