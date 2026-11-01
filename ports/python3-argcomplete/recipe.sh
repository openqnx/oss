# argcomplete: bash tab completion for argparse.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.7.2
TARBALL=https://files.pythonhosted.org/packages/46/bd/551ee6af426af84ca33e02622be722925c196608e9127d731ef17c47f06e/argcomplete-3.7.2-py3-none-any.whl
SHA256=6029205678bdd9c1c728a155f5f9ecf5812393f969eef58807641a2bc2aa5b19
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
