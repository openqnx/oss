# textparser: a text parser (tokenizer and grammar combinators) in pure
# Python; cantools reads DBC, SYM and other CAN database files with it.
# Pure Python: the published wheel is installed as released (scripts/lib/pywheel.sh).
VERSION=0.26.2
TARBALL=https://files.pythonhosted.org/packages/7d/cc/812fe3ae07a1917bb4cfb6dec5f6fc6e8825d65b4e77cd25bc52cd59230f/textparser-0.26.2-py3-none-any.whl
SHA256=e14ec4fd58d3515d897196a71f4ccf8d354f5c5c6b87910571b4d250de073c4b
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
