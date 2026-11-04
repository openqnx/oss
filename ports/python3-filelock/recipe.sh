# filelock: a platform independent file lock.
# Version 3.x: virtualenv 20 asks for filelock<4.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.32.7
TARBALL=https://files.pythonhosted.org/packages/15/df/31098c5aeb4d966b553641472bd55fcf5fdfac953549894b8a765ba44e91/filelock-3.32.7-py3-none-any.whl
SHA256=65ff0d0190ea42038b32bda4b77834fb05be2cad4c5b9b01aa4dfb3614536e52
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
