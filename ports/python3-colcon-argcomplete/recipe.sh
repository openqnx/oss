# colcon-argcomplete: completion for colcon command lines using argcomplete.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.3.3
TARBALL=https://files.pythonhosted.org/packages/f1/ef/131788328c9cb5d9882cde4d15dd5aabf2ed6e3d5f09657787c2430812e2/colcon_argcomplete-0.3.3-py3-none-any.whl
SHA256=0358ee95e7d7a7a5929d00bbe5b80138b1dcf7e5f7399ba221437a847c7b62d4
DEPS="python3 python3-argcomplete python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-argcomplete python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
# The wheel's .data/ (share/ files, scripts, headers) goes where pip puts it.
source "$WS/scripts/lib/pywheel-data.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_install_data; }
