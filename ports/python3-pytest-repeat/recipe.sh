# pytest-repeat: pytest plugin for repeating tests.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.9.4
TARBALL=https://files.pythonhosted.org/packages/73/d4/8b706b81b07b43081bd68a2c0359fe895b74bf664b20aca8005d2bb3be71/pytest_repeat-0.9.4-py3-none-any.whl
SHA256=c1738b4e412a6f3b3b9e0b8b29fcd7a423e50f87381ad9307ef6f5a8601139f3
DEPS="python3 python3-pytest"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-pytest"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
