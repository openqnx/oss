# colcon-ros: extension for colcon to support ROS packages.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.5.0
TARBALL=https://files.pythonhosted.org/packages/d5/d9/94442ea35bf6caf2f43c56372b450e36a3d6516c025babf2fda89ff6c126/colcon_ros-0.5.0-py3-none-any.whl
SHA256=21720baf112fdf3e1e96951bbf2aec0dc5b09afb884dc6cc50a2b419840749dd
DEPS="python3 python3-catkin-pkg python3-colcon-cmake python3-colcon-core python3-colcon-pkg-config python3-colcon-python-setup-py python3-colcon-recursive-crawl"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-catkin-pkg python3-colcon-cmake python3-colcon-core python3-colcon-pkg-config python3-colcon-python-setup-py python3-colcon-recursive-crawl"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
