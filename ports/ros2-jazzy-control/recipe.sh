VERSION=20260128
# ros2_control for ROS 2 Jazzy -- the controller manager, hardware interface
# (with mock_components' GenericSystem), controller interface, joint limits,
# transmissions, ros2controlcli, and ros2_controllers (joint state / IMU /
# force-torque / range / pose broadcasters, forward command, joint trajectory,
# diff drive, tricycle, Ackermann/bicycle steering, mecanum, PID, admittance,
# gripper and GPIO controllers) with what they are built on: control_msgs,
# control_toolbox, realtime_tools, kinematics_interface (KDL), filters,
# diagnostics, angles, ackermann_msgs, xacro, pal_statistics, backward_ros,
# generate_parameter_library with RSL and cpp_polyfills, and
# topic_based_hardware_interfaces. An overlay workspace on the ros2
# component, installed into its prefix (/usr/local/opt/ros/jazzy: source its
# setup.sh), built the way ports/ros2-rviz is.
#
# Upstream sources at the tags in control.repos -- the set and versions of
# qnx-ports/aports extra/ros2-jazzy-control (VERSION is its date); QNX changes
# in repo-patches/ (MAP says which repository each one is for).
# Not built: the GUIs (rqt_controller_manager, rqt_joint_trajectory_controller:
# rqt is not ported), kinematics_interface_pinocchio (Pinocchio is not
# ported), generate_parameter_library's example packages, and the
# ros2_control_demos (their launch files start RViz and Gazebo).
DEPS="ros2 eigen boost tinyxml2 fmt tl-expected"
# ros2: the ROS libraries and setup.sh it runs on (in the ROS prefix, not
# /usr/local/lib, so NEEDED does not show them); xacro and ros2controlcli are
# Python (the controller spawner locks with filelock); generate_parameter_library's
# Python runs at build time only.
RUNTIME_DEPS="ros2 python3-pyyaml python3-lxml python3-filelock"
BUILDSYS=custom
ROS_PREFIX="$PREFIX/opt/ros/jazzy"
source "$WS/scripts/lib/ros2-overlay.sh"
OVERLAY_REPOS="$RECIPE_DIR/ros2-jazzy-control/control.repos"
OVERLAY_PATCHES="$RECIPE_DIR/ros2-jazzy-control/repo-patches"
OVERLAY_IGNORE="src/ros-controls/ros2_control/rqt_controller_manager src/ros-controls/ros2_controllers/rqt_joint_trajectory_controller
src/ros-controls/kinematics_interface/kinematics_interface_pinocchio
src/PickNikRobotics/generate_parameter_library/example src/PickNikRobotics/generate_parameter_library/example_python
src/PickNikRobotics/generate_parameter_library/example_external"
# STACK_DETAILS_AUTO_DETECT off: backward_ros otherwise found the binutils
# component's static libbfd.a in staging and linked it (with -liberty, which
# is not there); stack traces print addresses only, as without libdw/libbfd.
OVERLAY_CMAKE_ARGS=( -DSTACK_DETAILS_AUTO_DETECT=OFF -DBoost_NO_BOOST_CMAKE=ON -DBoost_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include" )
OVERLAY_PIP=""
