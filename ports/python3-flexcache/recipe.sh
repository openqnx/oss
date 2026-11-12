# flexcache: caches the result of converting source files on disk (pint's unit
# registry uses it). Pure Python: the published wheel is installed as released
# (scripts/lib/pywheel.sh).
VERSION=0.3
TARBALL=https://files.pythonhosted.org/packages/27/cd/c883e1a7c447479d6e13985565080e3fea88ab5a107c21684c813dba1875/flexcache-0.3-py3-none-any.whl
SHA256=d43c9fea82336af6e0115e308d9d33a185390b8346a017564611f1466dcd2e32
DEPS="python3 python3-typing-extensions"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-typing-extensions"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
