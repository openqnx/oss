# pyflakes: a checker for errors in Python source (flake8's F codes).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=4.1.0
TARBALL=https://files.pythonhosted.org/packages/fd/52/98c9a0e4718abe887121279457bc7140115df22d714b7183880eb61e359f/pyflakes-4.1.0-py2.py3-none-any.whl
SHA256=b2e05e6bac8e49b67be2a6ffa63422c60f7193fc72e8180255907b41ec997dc1
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
