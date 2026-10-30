# pretend: stubs for tests.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.0.9
TARBALL=https://files.pythonhosted.org/packages/49/1f/3d4f0579913edd3ad5b23ad52fcc42531cb736ad52af2ba6c057da8785b6/pretend-1.0.9-py2.py3-none-any.whl
SHA256=e389b12b7073604be67845dbe32bf8297360ad9a609b24846fe15d86e0b7dc01
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
