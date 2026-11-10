VERSION=20260128
# MoveIt 2 for ROS 2 Jazzy, a core that builds and runs headless: moveit_core
# (robot model and state, kinematics, collision detection with FCL and
# Bullet, planning scene, trajectory processing with Ruckig and TOTG),
# the planners (OMPL, Pilz industrial motion planner, CHOMP, STOMP), the
# KDL/SRV/cached IK kinematics plugins, moveit_ros planning, warehouse,
# move_group and its capabilities, MoveItCpp and the planning interface
# (MoveGroupInterface), MoveIt Servo, benchmarks, and the controller
# managers (simple, ros2_control) -- with geometric_shapes, srdfdom,
# random_numbers, eigen_stl_containers, moveit_msgs, octomap_msgs,
# object_recognition_msgs, warehouse_ros and the STOMP library. An overlay
# on the ros2 and ros2-jazzy-control components, installed into the ROS
# prefix (/usr/local/opt/ros/jazzy), built as ports/ros2-rviz is.
#
# Upstream sources at the tags in moveit2.repos -- qnx-ports/aports extra/
# ros2-jazzy-moveit2's set and versions (VERSION is its date); QNX changes in
# repo-patches/ (MAP says where). Built against this tree's OMPL, Ruckig,
# FCL, libccd, octomap, Bullet, OSQP, qhull and assimp.
# Not built: the GUIs -- moveit_ros_visualization (RViz
# plugins), moveit_setup_assistant and its framework/plugins --,
# moveit_ros_robot_interaction (interactive markers, which come with
# ros2-rviz), moveit_ros_perception (OpenGL depth-image filter, octomap
# updater; QNX has no desktop GL) and moveit_py (Python bindings).
DEPS="ros2 ros2-jazzy-control ompl ruckig fcl libccd octomap bullet3 osqp qhull assimp boost eigen tinyxml2 fmt"
RUNTIME_DEPS="ros2 ros2-jazzy-control"
BUILDSYS=custom
ROS_PREFIX="$PREFIX/opt/ros/jazzy"
source "$WS/scripts/lib/ros2-overlay.sh"
OVERLAY_REPOS="$RECIPE_DIR/ros2-jazzy-moveit2/moveit2.repos"
OVERLAY_PATCHES="$RECIPE_DIR/ros2-jazzy-moveit2/repo-patches"
OVERLAY_IGNORE="src/moveit/moveit2/moveit_ros/visualization src/moveit/moveit2/moveit_setup_assistant
src/moveit/moveit2/moveit_ros/robot_interaction src/moveit/moveit2/moveit_ros/perception src/moveit/moveit2/moveit_py
src/moveit/moveit2/moveit_ros/moveit_ros src/moveit/moveit2/moveit src/moveit/moveit2/moveit_runtime
src/moveit/moveit2/moveit_planners/moveit_planners src/moveit/moveit2/moveit_kinematics/test"
OVERLAY_CMAKE_ARGS=( -DBoost_NO_BOOST_CMAKE=ON -DBoost_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include" -DWITH_OPENGL=OFF -DBUILD_TESTING=OFF )
OVERLAY_PIP=""
