# coverage: code coverage measurement for Python (the pure-Python tracer; no C tracer).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=7.17.4
TARBALL=https://files.pythonhosted.org/packages/fb/b5/3039295337fcda9dfcaf6d4d294246c4400587d8f4a23b29b001c5cea588/coverage-7.17.4-py3-none-any.whl
SHA256=0f9547c30aeef70904ede391f4cb43d2ddd0a0d2d8fb4a96c75cc422f64afaac
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
