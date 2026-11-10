# pydocstyle: the PEP 257 docstring checker (D1xx-D4xx; flake8-docstrings).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=6.3.0
TARBALL=https://files.pythonhosted.org/packages/36/ea/99ddefac41971acad68f14114f38261c1f27dac0b3ec529824ebc739bdaa/pydocstyle-6.3.0-py3-none-any.whl
SHA256=118762d452a49d6b05e194ef344a55822987a462831ade91ec5c06fd2169d019
DEPS="python3 python3-snowballstemmer"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-snowballstemmer"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
