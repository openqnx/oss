# flake8-comprehensions: flake8 plugin for better list/set/dict
# comprehensions (C4xx).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.17.0
TARBALL=https://files.pythonhosted.org/packages/39/bd/d6739d685fdd79349aa51c37bdedc0d8eab6ae9c6e6ed2ca935b3f88210d/flake8_comprehensions-3.17.0-py3-none-any.whl
SHA256=3943a9c6f2593c3bc5cc64106c2f89d63c6ecd49c8343597f8257b8fcfc8b0a2
DEPS="python3 python3-flake8"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-flake8"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
