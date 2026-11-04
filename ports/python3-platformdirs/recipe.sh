# platformdirs: the platform's user and site data/config/cache directories.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=4.14.3
TARBALL=https://files.pythonhosted.org/packages/93/41/f0aa003909ee77a9e7e55f8659abb635ca23ab0d022a449bbb18da87a1ef/platformdirs-4.14.3-py3-none-any.whl
SHA256=cba2d05fe5e18b26b1fa6f376b8e6234c012abe1f8696c486fface7d0054e931
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
