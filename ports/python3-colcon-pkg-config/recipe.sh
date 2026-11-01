# colcon-pkg-config: extension for colcon adding a variable to find pkg-config files.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.1.0
TARBALL=https://files.pythonhosted.org/packages/75/28/d738200c7eed21cd78df901fa78818618f5b85dd39ea7c3905b367fcb033/colcon_pkg_config-0.1.0-py3-none-any.whl
SHA256=3a79aa429dc8cfc534b35a80fe04483b9fd730b436cfd5b093aed2934c804206
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
