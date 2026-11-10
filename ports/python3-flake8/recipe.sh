# flake8: the Python linter that runs pyflakes, pycodestyle and mccabe
# and its plugins.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=7.5.0
TARBALL=https://files.pythonhosted.org/packages/5d/57/a3adc5f380f50ef3d01014cb9d3b8c17b25de9b89def20d559def4fc3c4e/flake8-7.5.0-py2.py3-none-any.whl
SHA256=637ea1a443a78ed6ee461725cdf71ca93bc5cde0abf25436dbebdc3880b4e078
DEPS="python3 python3-mccabe python3-pycodestyle python3-pyflakes"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-mccabe python3-pycodestyle python3-pyflakes"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
