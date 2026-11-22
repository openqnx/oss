# mypy_extensions: type system extensions for mypy (TypedDict-era helpers, trait, mypyc_attr).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.2.0
TARBALL=https://files.pythonhosted.org/packages/b5/99/7b1adac7e97ce7bbda34792dc6e9b2f5cc5cffd1875883f78a818b2a2687/mypy_extensions-1.2.0-py3-none-any.whl
SHA256=993e7a20aa5671eb41a7132cf555af30062da99729601601b9a8382b11cae768
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
