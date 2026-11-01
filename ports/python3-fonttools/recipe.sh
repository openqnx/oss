# fonttools: read, write and manipulate font files (TrueType, OpenType, WOFF).
# The platform-independent wheel: the optional Cython accelerators (cu2qu,
# qu2cu, the feaLib lexer) fall back to their pure-Python implementations.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=4.68.0
TARBALL=https://files.pythonhosted.org/packages/35/74/4ea331ecc65ba05a58b1e4e8ccd604c8a15565f876dfbf93fc2a2ea2e22b/fonttools-4.68.0-py3-none-any.whl
SHA256=d76f8fdcb6936f5e1045a0c9dd721a4bf11112e60d1e243bfa1d39f06a8b3104
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
# The wheel's .data/ (share/ files, scripts, headers) goes where pip puts it.
source "$WS/scripts/lib/pywheel-data.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_install_data; }
