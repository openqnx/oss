# flake8-builtins: flake8 plugin for names that shadow Python builtins (A00x).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.2.0
TARBALL=https://files.pythonhosted.org/packages/b6/02/c7614f54d3fde51b8a20d91e14b012a01c78c2a481d36b5e8e40535a3d0b/flake8_builtins-3.2.0-py3-none-any.whl
SHA256=192a00630f183b8e7b881e98def18aa20124581dde3a3e7be7a01a6effe3504b
DEPS="python3 python3-flake8"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-flake8"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
