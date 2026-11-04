# pyproject-metadata: PEP 621 metadata from pyproject.toml.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.12.1
TARBALL=https://files.pythonhosted.org/packages/b9/8e/d883718e872abc214ba053f60f7e6edc2abd5ffbd1544beb01a9c92ddd3c/pyproject_metadata-0.12.1-py3-none-any.whl
SHA256=f7162d580a96386a8eb096da06215f981f547d1490f03055ef99e323bc2da427
DEPS="python3 python3-packaging"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-packaging"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
