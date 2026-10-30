# pytokens: a fast, spec-compliant Python 3.12+ tokenizer.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.4.1
TARBALL=https://files.pythonhosted.org/packages/c6/78/397db326746f0a342855b81216ae1f0a32965deccfd7c830a2dbc66d2483/pytokens-0.4.1-py3-none-any.whl
SHA256=26cef14744a8385f35d0e095dc8b3a7583f6c953c2e3d269c7f82484bf5ad2de
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
