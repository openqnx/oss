# colcon-notification: extension for colcon to provide status notifications.
# Its desktop notifications need notify2 on Linux only (sys_platform
# "linux"); on QNX it updates the terminal title.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.3.3
TARBALL=https://files.pythonhosted.org/packages/8e/85/b268abde9b36656211d3ed10745d352d0e8dd492ff552b66847856a97fff/colcon_notification-0.3.3-py3-none-any.whl
SHA256=c3cffd9f8bbd4e673307fb3d4d06d624d0d9023bad1c12c3fea2dd3f9b641ad9
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
# The wheel's .data/ (share/ files, scripts, headers) goes where pip puts it.
source "$WS/scripts/lib/pywheel-data.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_install_data; }
