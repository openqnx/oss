# wrapt: transparent object proxies and decorators that keep a function's
# signature (python-can's dependency). The released py3-none-any wheel is
# the pure-Python implementation (without the optional C extension); it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=2.5.2
TARBALL=https://files.pythonhosted.org/packages/77/8b/1ea57b936b70f98e8e7620a352578096d778dbee95b262f73c066d210ffb/wrapt-2.5.2-py3-none-any.whl
SHA256=61eb54e9540ea522c2c4752a57549a83d3c67e6704ed708403d28e067aaf90f0
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
