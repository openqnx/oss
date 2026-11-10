# flake8-deprecated: flake8 plugin for deprecated method names (D001).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=2.4.0
TARBALL=https://files.pythonhosted.org/packages/20/7d/683eace5cb115b7cc91a49a30d23a857cdd5165b63f6f0eb2ebd8196b970/flake8_deprecated-2.4.0-py3-none-any.whl
SHA256=a5d4608873b5c4d2bef0f1653f125b1603f46a016f30e95d4f58c97731d3aa17
DEPS="python3 python3-flake8"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-flake8"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
