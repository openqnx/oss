# hatch-fancy-pypi-readme: README composition for hatchling.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers (qnx_py_scripts).
VERSION=25.1.0
TARBALL=https://files.pythonhosted.org/packages/58/b4/ad71e051b34f1935783de37beffee16e343ff2514b204dc515f9e939d4af/hatch_fancy_pypi_readme-25.1.0-py3-none-any.whl
SHA256=ce0134c40d63d874ac48f48ccc678b8f3b62b8e50e9318520d2bffc752eedaf3
DEPS="python3 python3-hatchling"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-hatchling"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
