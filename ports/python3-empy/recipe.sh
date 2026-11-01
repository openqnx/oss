# empy: a templating system for Python.
# Pure Python, published only as a source distribution: built into a
# platform-independent wheel on the build machine (scripts/lib/pywheel.sh).
VERSION=4.2.1
TARBALL=https://files.pythonhosted.org/packages/d1/82/b8b39f837baa80b1bf659beb64c1c8af66f2aa7437169d72a4e1fb0ed3df/empy-4.2.1.tar.gz
SHA256=b831d642fca95507820b53774c051803142b8441b8c42bf800011da06ba4241b
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
# The wheel's .data/ (share/ files, scripts, headers) goes where pip puts it.
source "$WS/scripts/lib/pywheel-data.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_build() { qnx_py_wheel_pure; }
do_install() { qnx_py_install && qnx_py_install_data; }
