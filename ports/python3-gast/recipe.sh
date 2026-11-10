# gast: a generic, version independent Python AST (pythran, beniget).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.7.0
TARBALL=https://files.pythonhosted.org/packages/1d/33/f1c6a276de27b7d7339a34749cc33fa87f077f921969c47185d34a887ae2/gast-0.7.0-py3-none-any.whl
SHA256=99cbf1365633a74099f69c59bd650476b96baa5ef196fec88032b00b31ba36f7
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
