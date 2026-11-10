# cppy: C++ headers that make writing Python C extensions easier (the
# build dependency of kiwisolver and atom); the headers are installed with it.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.3.1
TARBALL=https://files.pythonhosted.org/packages/8b/3c/823fda3f226d84f983e48005527a6dd2dc365220ac4f098b770779d9f4a1/cppy-1.3.1-py3-none-any.whl
SHA256=7ca132b6ef6187384738804bd3a453d4eab7e3274df6dcd35e5d92aae3404717
DEPS="python3 python3-setuptools"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-setuptools"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
