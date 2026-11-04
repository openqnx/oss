# virtualenv: isolated Python environments.
# Version 20.39.1: virtualenv 21 needs python-discovery and packaging>=26.3.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers (qnx_py_scripts).
VERSION=20.39.1
TARBALL=https://files.pythonhosted.org/packages/f6/7f/89d7e8a397752826dbbb81127dfb8feab8286f4f1f91a820f646f04368fb/virtualenv-20.39.1-py3-none-any.whl
SHA256=a00332e0b089ba64edefbf94ef34e6db33f45fe5184df0652cf0b754dcd36190
DEPS="python3 python3-distlib python3-filelock python3-platformdirs"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-distlib python3-filelock python3-platformdirs"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
