# rosdistro: reads ROS distribution files (index.yaml, distribution.yaml)
# and their caches -- the ROS build farm and rosdep's data.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=1.1.0
TARBALL=https://files.pythonhosted.org/packages/12/74/ef303ec980ef962ffe548fa36cb138bb399d1d1315731ba7bb81da85f7e0/rosdistro-1.1.0-py3-none-any.whl
SHA256=a10d0852fcef15b2b9c92a7b4d553481f660e4e672eaeb1138ad077ffe9a2a38
DEPS="python3 python3-packaging python3-pyyaml python3-setuptools python3-catkin-pkg python3-rospkg"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-packaging python3-pyyaml python3-setuptools python3-catkin-pkg python3-rospkg"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
