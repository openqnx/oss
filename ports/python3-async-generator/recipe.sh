# async_generator: async generators and async context managers (backport helpers).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.10
TARBALL=https://files.pythonhosted.org/packages/71/52/39d20e03abd0ac9159c162ec24b93fbcaa111e8400308f2465432495ca2b/async_generator-1.10-py3-none-any.whl
SHA256=01c7bf666359b4967d2cda0000cc2e4af16a0ae098cbffcb8472fb9e8ad6585b
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
