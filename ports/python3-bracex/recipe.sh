# bracex: Bash-style brace expansion ({a,b}, {1..10}) for Python.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.2.1
TARBALL=https://files.pythonhosted.org/packages/47/ae/30dd8b027b4032b27e81364de5b74bfbef50b3c84c6aa0ead351bbb3749c/bracex-3.2.1-py3-none-any.whl
SHA256=e8622d46b60ef6062b4091033f97a304e1b205e70c0285743c5038b73886a47e
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
