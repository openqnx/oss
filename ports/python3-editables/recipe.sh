# editables: editable installs for build backends.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.6
TARBALL=https://files.pythonhosted.org/packages/cc/0e/d095533037fc9468f947b19f2da53f869804632b508808de41e86e801797/editables-0.6-py3-none-any.whl
SHA256=d70e4698078a1d033e7786d9c64e5be070d058a67c21417024d38a58ac20aa43
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
