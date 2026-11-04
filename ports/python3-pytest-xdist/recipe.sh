# pytest-xdist: runs pytest tests in parallel worker processes (-n).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.8.0
TARBALL=https://files.pythonhosted.org/packages/ca/31/d4e37e9e550c2b92a9cbc2e4d0b7420a27224968580b5a447f420847c975/pytest_xdist-3.8.0-py3-none-any.whl
SHA256=202ca578cfeb7370784a8c33d6d05bc6e13b4f25b5053c30a152269fd10f0b88
DEPS="python3 python3-execnet python3-pytest"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-execnet python3-pytest"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
