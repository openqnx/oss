# flexparser: a line and block oriented parser built on dataclasses and typing
# (pint parses its unit definition files with it). Pure Python: the published
# wheel is installed as released (scripts/lib/pywheel.sh).
VERSION=0.4
TARBALL=https://files.pythonhosted.org/packages/fe/5e/3be305568fe5f34448807976dc82fc151d76c3e0e03958f34770286278c1/flexparser-0.4-py3-none-any.whl
SHA256=3738b456192dcb3e15620f324c447721023c0293f6af9955b481e91d00179846
DEPS="python3 python3-typing-extensions"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-typing-extensions"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
