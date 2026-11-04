# flit_core: the PEP 517 build backend of Flit.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=4.2.0
TARBALL=https://files.pythonhosted.org/packages/d9/f0/e546c28b937f094b354ca29b9060c5f1dd5043a332b2905049a51420b7a2/flit_core-4.2.0-py3-none-any.whl
SHA256=2bee22dc8ba89c2e35b4e1ad575e596aac945c85672dd8f3cad21b629036e044
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
