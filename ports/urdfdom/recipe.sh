# urdfdom: the URDF (Unified Robot Description Format) parser of ROS, on
# urdfdom_headers' model classes, console_bridge for its messages and
# tinyxml2 for the XML.
VERSION=4.0.3; GIT=https://github.com/ros/urdfdom.git; REF=4.0.3
DEPS="urdfdom_headers console_bridge tinyxml2"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF )
# 4.0.x is the series for urdfdom_headers 1.x (the version ported here);
# urdfdom 5 and 6 need urdfdom_headers 2.1 and 3.0.
