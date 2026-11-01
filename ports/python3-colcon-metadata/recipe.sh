# colcon-metadata: extension for colcon to read package metadata from files.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.3.1
TARBALL=https://files.pythonhosted.org/packages/98/f8/d2ba851cedc174c0fe621c0fc26af0e3f1094d0ee2c9ff9d2f7875800798/colcon_metadata-0.3.1-py3-none-any.whl
SHA256=3b4f474ba56e17bb1848165b9bfa518c98f0d6379b325c899f33b46a78d0f609
DEPS="python3 python3-colcon-core python3-pyyaml"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core python3-pyyaml"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
