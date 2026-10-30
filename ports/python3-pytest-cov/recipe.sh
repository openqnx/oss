# pytest-cov: coverage measurement from pytest (--cov).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=7.1.0
TARBALL=https://files.pythonhosted.org/packages/9d/7a/d968e294073affff457b041c2be9868a40c1c71f4a35fcc1e45e5493067b/pytest_cov-7.1.0-py3-none-any.whl
SHA256=a0461110b7865f9a271aa1b51e516c9a95de9d696734a2f71e3e78f46e1d4678
DEPS="python3 python3-pytest python3-coverage python3-pluggy"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-pytest python3-coverage python3-pluggy"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
