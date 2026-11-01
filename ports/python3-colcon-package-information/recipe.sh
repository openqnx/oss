# colcon-package-information: extension for colcon to output package information.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.4.1
TARBALL=https://files.pythonhosted.org/packages/1e/ee/a6b0ffdc06c67f3ab740c3bf8efaf520c7adb3b4bf189b447e90101ff838/colcon_package_information-0.4.1-py3-none-any.whl
SHA256=951c5e596cd0cf5cc82965fd23f2d8c1be78f7ac0cc409a2ee552e4ef0f374d3
DEPS="python3 python3-colcon-core python3-packaging"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core python3-packaging"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
