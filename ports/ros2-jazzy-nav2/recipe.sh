VERSION=20260128
# Nav2, the ROS 2 Jazzy navigation stack -- lifecycle-managed servers for
# maps (nav2_map_server), localisation (nav2_amcl), costmaps (nav2_costmap_2d
# with its layers), global planners (NavFn, Smac 2D/Hybrid-A*/lattice, Theta*),
# controllers (DWB, Regulated Pure Pursuit, MPPI, rotation shim, graceful),
# smoothers, behaviours, waypoint following, collision monitor, velocity
# smoother, docking (opennav_docking) and the BehaviorTree.CPP 4 navigator
# (nav2_bt_navigator) -- with robot_localization (EKF/UKF), bond_core and
# geographic_info. An overlay workspace on the ros2 and ros2-jazzy-control
# components (diagnostics, angles), installed into the ROS prefix
# (/usr/local/opt/ros/jazzy), built as ports/ros2-rviz is.
#
# Upstream sources at the tags in nav2.repos -- qnx-ports/aports extra/
# ros2-jazzy-nav2's set and versions (VERSION is its date), without
# vision_opencv (the ros2 component has it) and the TurtleBot simulation
# (Gazebo); QNX changes in repo-patches/ (MAP says where).
# Not built: nav2_rviz_plugins (RViz panels: they would tie Nav2 to the
# ros2-rviz/Qt 6 GUI), nav2_system_tests, and slam_toolbox (its node needs
# interactive_markers, which ships with ros2-rviz, and an RViz plugin).
DEPS="ros2 ros2-jazzy-control boost eigen tinyxml2 geographiclib graphicsmagick xtensor xtl xsimd nanoflann nlohmann_json libzmq cppzmq sqlite libuuid ceres-solver suitesparse ompl glog opencv"
RUNTIME_DEPS="ros2 ros2-jazzy-control"
BUILDSYS=custom
ROS_PREFIX="$PREFIX/opt/ros/jazzy"
source "$WS/scripts/lib/ros2-overlay.sh"
OVERLAY_REPOS="$RECIPE_DIR/ros2-jazzy-nav2/nav2.repos"
OVERLAY_PATCHES="$RECIPE_DIR/ros2-jazzy-nav2/repo-patches"
OVERLAY_IGNORE="src/ros-navigation/navigation2/nav2_rviz_plugins src/ros-navigation/navigation2/nav2_system_tests"
OVERLAY_CMAKE_ARGS=( -DBoost_NO_BOOST_CMAKE=ON -DBoost_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include" -DBTCPP_EXAMPLES=OFF -DBTCPP_UNIT_TESTS=OFF -DBTCPP_GROOT_INTERFACE=ON -DBTCPP_SQLITE_LOGGING=ON )
OVERLAY_PIP=""
