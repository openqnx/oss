VERSION=4.3.0; GIT=https://github.com/borglab/gtsam.git; REF=4.3.0
DEPS="boost eigen"
# GTSAM_BUILD_WITH_WERROR (on by default when GTSAM is the top-level
# project) turns every warning into an error. With QNX's libc++, GCC 12 then
# stops at a false positive inside <new> (-Walloc-size-larger-than on a
# std::vector copy inlined into HybridNonlinearFactor.cpp: a size it cannot
# prove is never reached). A release build keeps the warnings as warnings.
CMAKE_ARGS_EXTRA=( -DGTSAM_BUILD_TESTS=OFF -DGTSAM_BUILD_EXAMPLES_ALWAYS=OFF -DGTSAM_BUILD_PYTHON=OFF
                   -DGTSAM_WITH_TBB=OFF -DGTSAM_USE_SYSTEM_EIGEN=ON -DGTSAM_BUILD_UNSTABLE=OFF -DGTSAM_BUILD_WITH_WERROR=OFF
                   -DGTSAM_BUILD_WITH_MARCH_NATIVE=OFF -DBoost_NO_BOOST_CMAKE=OFF -DBOOST_ROOT="$STAGE$PREFIX" )
