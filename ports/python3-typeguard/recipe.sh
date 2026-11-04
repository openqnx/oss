# typeguard: run-time type checking.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=4.6.0
TARBALL=https://files.pythonhosted.org/packages/8f/eb/461d5f167b6f5c7d97696f397c82f82e3480e003fce3f0a1cd1dd26e2eb2/typeguard-4.6.0-py3-none-any.whl
SHA256=79878165bb86f2cf5d41d159a0ff1792a796cf496882d2fe1b1c6c7049b9cdd7
DEPS="python3 python3-typing-extensions"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-typing-extensions"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
