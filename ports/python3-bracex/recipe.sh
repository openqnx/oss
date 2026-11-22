# bracex: Bash-style brace expansion ({a,b}, {1..10}) for Python.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.2.4
TARBALL=https://files.pythonhosted.org/packages/a1/5b/7b404360cc0946686cae68ee564d859a7c57379d190c88edec18fcdc9396/bracex-3.2.4-py3-none-any.whl
SHA256=82e9e738b63fca213c33211d30b0b1ca330896a1091736e78da2c1bcda93ab30
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
