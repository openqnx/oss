# flake8-docstrings: runs pydocstyle as a flake8 plugin (D codes).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.7.0
TARBALL=https://files.pythonhosted.org/packages/3f/7d/76a278fa43250441ed9300c344f889c7fb1817080c8fb8996b840bf421c2/flake8_docstrings-1.7.0-py2.py3-none-any.whl
SHA256=51f2344026da083fc084166a9353f5082b01f72901df422f74b4d953ae88ac75
DEPS="python3 python3-flake8 python3-pydocstyle"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-flake8 python3-pydocstyle"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
