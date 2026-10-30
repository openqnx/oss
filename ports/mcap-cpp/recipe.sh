# mcap-cpp: the C++ library for MCAP, the container format for timestamped
# multi-channel messages (ROS 2 bags, robotics logging). Header only; it
# reads and writes compressed chunks with lz4 and zstd.
VERSION=2.1.3; GIT=https://github.com/foxglove/mcap.git; REF=releases/cpp/v2.1.3
DEPS="lz4 zstd"
BUILDSYS=custom
do_configure() { :; }
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/include" && cp -a "$S/cpp/mcap/include/mcap" "$STAGE$PREFIX/include/"; }
