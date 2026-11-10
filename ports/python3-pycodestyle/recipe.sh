# pycodestyle: the PEP 8 style checker (flake8's E/W codes).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=2.15.0
TARBALL=https://files.pythonhosted.org/packages/56/e2/480bcdb153feecde4d736b18c908c7eee4846f11d7547d2e0c71499af630/pycodestyle-2.15.0-py2.py3-none-any.whl
SHA256=12fd2f73c7b8ee8845a0431111df8faf4c1a07d6e64e2ee7f0c74014dab14181
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
