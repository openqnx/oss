# meson-python: the PEP 517 build backend for Meson projects.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.22.1
TARBALL=https://files.pythonhosted.org/packages/96/62/0c1cf20fe158b0e623e78acbf5fb6de5f1410a408f65f727e4090497627b/meson_python-0.22.1-py3-none-any.whl
SHA256=258fde948cf45aab5df23fcc9e17a9050ee73d4e1002fe6cf86519f61c414778
DEPS="python3 meson python3-packaging python3-pyproject-metadata"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 meson python3-packaging python3-pyproject-metadata"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
