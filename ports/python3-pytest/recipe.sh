# pytest: the Python test framework.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=9.2.0
TARBALL=https://files.pythonhosted.org/packages/51/39/1f5d8c8877a0fa327db959c19341d4be61660de1ab6b7391f9eb805bcd67/pytest-9.2.0-py3-none-any.whl
SHA256=07acdcf88ec174f907dc01af70bf52d49b014bbab03faeeade31c96b18c2e588
DEPS="python3 python3-iniconfig python3-packaging python3-pluggy python3-pygments"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-iniconfig python3-packaging python3-pluggy python3-pygments"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
