# OMPL, the Open Motion Planning Library: sampling-based motion planners (RRT,
# RRT*, PRM, KPIECE, BIT*, ...) over state spaces such as SE(2), SE(3) and
# real vector spaces. MoveIt 2 plans with it (moveit_planners_ompl).
# The C++ library only: the Python bindings (Py++/castxml), demos and tests
# are not built. PYTHON_EXEC names the build host's interpreter: OMPL's own
# FindPython otherwise found the staged QNX python3, could not run it and
# stopped ("list GET given empty list").
VERSION=1.7.0; GIT=https://github.com/ompl/ompl.git; REF=1.7.0
DEPS="boost eigen"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DOMPL_BUILD_PYBINDINGS=OFF -DOMPL_BUILD_PYTESTS=OFF -DOMPL_BUILD_DEMOS=OFF -DOMPL_BUILD_TESTS=OFF
                   -DOMPL_REGISTRATION=OFF -DPYTHON_EXEC=/usr/bin/python3 -DCMAKE_DISABLE_FIND_PACKAGE_spot=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_Triangle=ON -DCMAKE_DISABLE_FIND_PACKAGE_flann=ON -DCMAKE_DISABLE_FIND_PACKAGE_ODE=ON )
