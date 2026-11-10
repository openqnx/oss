# vcstool: works on many version-controlled repositories at once (vcs import
# of a .repos file, vcs status, pull, ...): the ROS 2 workspace tool.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=0.3.0
TARBALL=https://files.pythonhosted.org/packages/6c/d5/4aca2c05481a0fb74bd2660b14b0dd0ea975e4f38bc150511a64c55af986/vcstool-0.3.0-py3-none-any.whl
SHA256=ad73309e83b67344efb1f6cf9f556b2d75e297b4b137f643378ba75f930a6ecb
DEPS="python3 python3-pyyaml python3-setuptools"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-pyyaml python3-setuptools"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
