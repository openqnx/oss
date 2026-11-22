# beniget: def-use chains and scopes over a gast tree (pythran's analyses).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.5.1
TARBALL=https://files.pythonhosted.org/packages/33/40/6840c2e7d29b791702202c6e9da224cc189fdb3f28a72de43fa328c29753/beniget-0.5.1-py3-none-any.whl
SHA256=c57375e04fa8c812d2f3650900bbb393615810e49673e9a62de329e1b3696938
DEPS="python3 python3-gast"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-gast"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
