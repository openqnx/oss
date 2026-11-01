# docutils: Python documentation utilities.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.23
TARBALL=https://files.pythonhosted.org/packages/32/91/30151a39f7570f448ed84529390628a651d7f27c87d73c9b887f8189695e/docutils-0.23-py3-none-any.whl
SHA256=25d013af9bf23bc1c7b2b093dff4208166c53a94786c9e447808335ef1185fea
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
