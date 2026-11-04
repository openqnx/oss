# installer: installs Python wheels.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.1.0
TARBALL=https://files.pythonhosted.org/packages/d7/7a/63547a792a12148882205d79e07644091d5b7690f40c2bb9d41452a8bdc2/installer-1.1.0-py3-none-any.whl
SHA256=f55f5728ff61d3dd54dbc5a397cf3f7307cb64b180c20b72afbebf01c9e2c8a0
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
