# coverage: code coverage measurement for Python (the pure-Python tracer; no C tracer).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=7.17.0
TARBALL=https://files.pythonhosted.org/packages/c9/17/48032919343afa2906112d458aceda2043afdb24a2ee8eb4c60904d8ddf1/coverage-7.17.0-py3-none-any.whl
SHA256=bac487dce0d2aff28ddf1a533a916f771416eee0e493960f611e49037b878e9d
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
