# filelock: a platform independent file lock.
# Version 3.x: virtualenv 20 asks for filelock<4.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=4.2.12
TARBALL=https://files.pythonhosted.org/packages/8a/c9/eb6b6528c7edb958d4c24ac47a8b94113d9de36e4cfc6eecd599b1a4f349/filelock-4.2.12-py3-none-any.whl
SHA256=21decfec02f116ed4c954433cdb31f28c3b346b784543177b6d7059da2702ee9
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
