# mccabe: McCabe cyclomatic complexity checker (a flake8 plugin, C90x).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.7.0
TARBALL=https://files.pythonhosted.org/packages/27/1a/1f68f9ba0c207934b35b86a8ca3aad8395a3d6dd7921c0686e23853ff5a9/mccabe-0.7.0-py2.py3-none-any.whl
SHA256=6c2d30ab6be0e4a46919781807b4f0d834ebdd6c6e3dca0bda5a15f863427b6e
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
