# Pexpect: drives interactive child programs through a pseudo terminal.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=4.9.0
TARBALL=https://files.pythonhosted.org/packages/9e/c3/059298687310d527a58bb01f3b1965787ee3b40dce76752eda8b44e9a2c5/pexpect-4.9.0-py2.py3-none-any.whl
SHA256=7236d1e080e4936be2dc3e326cec0af72acf9212a7e1d060210e70a47e253523
DEPS="python3 python3-ptyprocess"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-ptyprocess"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
