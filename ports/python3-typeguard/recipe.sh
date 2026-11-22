# typeguard: run-time type checking.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=4.6.1
TARBALL=https://files.pythonhosted.org/packages/04/d5/ab99dd595bd3cc307346c9c644556b627ca0bd273d3ee23041e8ac4f20dd/typeguard-4.6.1-py3-none-any.whl
SHA256=aca1f365124873d5325b18aad1a3689d62f7184abe32d899413562cf51729d02
DEPS="python3 python3-typing-extensions"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-typing-extensions"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
