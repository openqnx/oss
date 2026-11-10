VERSION=1.2.3
# Lanelet2: the map framework for automated driving (Autoware's HD map
# format) -- lanelets, areas, regulatory elements and their geometry
# (lanelet2_core), OSM map reading/writing (lanelet2_io), projections
# between lat/lon and the map frame (lanelet2_projection: UTM, local
# transverse Mercator with GeographicLib), traffic rules, routing graphs and
# shortest routes (lanelet2_routing), map validation and the example maps
# (lanelet2_maps). Built as ROS 2 packages with KIT-MRT's mrt_cmake_modules,
# as an overlay on the ros2 component, installed into its prefix
# (/usr/local/opt/ros/jazzy). Not built: lanelet2_python and
# lanelet2_examples (Boost.Python, which the boost component leaves out).
# QNX ships this as Autoware.Auto's lanelet2_vendor (gitlab.com/qnx), whose
# QNX patch (std::localeconv) this tree's toolchain does not need.
DEPS="ros2 boost eigen geographiclib pugixml"
RUNTIME_DEPS="ros2"
BUILDSYS=custom
ROS_PREFIX="$PREFIX/opt/ros/jazzy"
source "$WS/scripts/lib/ros2-overlay.sh"
OVERLAY_REPOS="$RECIPE_DIR/lanelet2/lanelet2.repos"
OVERLAY_PATCHES="$RECIPE_DIR/lanelet2/repo-patches"
OVERLAY_IGNORE="src/fzi-forschungszentrum-informatik/Lanelet2/lanelet2_python src/fzi-forschungszentrum-informatik/Lanelet2/lanelet2_examples"
OVERLAY_CMAKE_ARGS=( -DBoost_NO_BOOST_CMAKE=ON -DBoost_INCLUDE_DIR="$STAGE_SYSROOT$PREFIX/include" -DMRT_ENABLE_COVERAGE=OFF )
OVERLAY_PIP=""
