# catkin_pkg: the catkin package library (package.xml parsing; colcon-ros).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.1.1
TARBALL=https://files.pythonhosted.org/packages/3f/53/b5e80bac66d0db921ddbbe3c8d37198455c19302a29da1a434708c35c79d/catkin_pkg-1.1.1-py3-none-any.whl
SHA256=491d5b5f65dddfba801c5b75f7fb645f0f187a02bedc01a9fc24dcfbf1039211
DEPS="python3 python3-dateutil python3-docutils python3-packaging python3-pyparsing"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-dateutil python3-docutils python3-packaging python3-pyparsing"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
