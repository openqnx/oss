# beniget: def-use chains and scopes over a gast tree (pythran's analyses).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.5.0
TARBALL=https://files.pythonhosted.org/packages/66/24/0bef8a8bbf403b536fb2368b6c3fb5f6669b3ef7fdbc2456a3affdd19836/beniget-0.5.0-py3-none-any.whl
SHA256=909efcd083af1ac4a5db311f57c415df8cb05c4307eda345edeafac3cd930043
DEPS="python3 python3-gast"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-gast"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
