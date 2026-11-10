# flake8-class-newline: flake8 plugin requiring a blank line after a class
# definition (CNL100).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.6.0
TARBALL=https://files.pythonhosted.org/packages/94/35/e62c4643382c7c21836cae115604d6c397dbaeae08c8853e5c46e8657cd9/flake8_class_newline-1.6.0-py3-none-any.whl
SHA256=130a6991e8c58ec9a6380d273e449e61bcd974b446c97f1e16493f6350fa8d35
DEPS="python3 python3-flake8"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-flake8"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
