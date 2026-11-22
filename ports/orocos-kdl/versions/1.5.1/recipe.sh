VERSION=1.5.1; GIT=https://github.com/orocos/orocos_kinematics_dynamics.git; REF=v1.5.1
DEPS="eigen"
SUBDIR=orocos_kdl
CMAKE_ARGS_EXTRA=( -DENABLE_TESTS=OFF -DENABLE_EXAMPLES=OFF -DEigen3_DIR="$STAGE$PREFIX/share/eigen3/cmake" )
