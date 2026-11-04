# tomli: a TOML parser.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=2.5.0
TARBALL=https://files.pythonhosted.org/packages/60/3f/3e3f8fd0919249b0200c80fbc4f9a1e70be19f9883da71dfb7f8b9ab8aca/tomli-2.5.0-py3-none-any.whl
SHA256=32a7b79ac57a2e83670ce329ccf675798bc5a2094783a63676866b70503f2e2b
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
