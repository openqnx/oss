# rospkg: ROS package library (finding packages and stacks, rosversion).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=1.6.3
TARBALL=https://files.pythonhosted.org/packages/15/63/327d25d91af32209b394abe5626706c4b7ad5a4812cc464a4e3a2be6916f/rospkg-1.6.3-py3-none-any.whl
SHA256=162dd71eccdd7ead29f5607e635b3e0793ced84952e5cf217e38b9a905855323
DEPS="python3 python3-catkin-pkg python3-pyyaml python3-distro"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-catkin-pkg python3-pyyaml python3-distro"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
