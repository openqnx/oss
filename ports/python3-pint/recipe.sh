# pint: physical quantities (a value and its unit), unit conversion and
# dimensional analysis. 0.24.x is the series python3-obd pins (pint==0.24.*).
# Pure Python: the published wheel is installed as released
# (scripts/lib/pywheel.sh).
VERSION=0.26.2
TARBALL=https://files.pythonhosted.org/packages/b7/d5/01ea71591a29e88926582e3ca7ed32f8b933e9ebc99187a7ab100587242d/pint-0.26.2-py3-none-any.whl
SHA256=c94124cdb19baa1a84bb65c8bb9a620a9636b9b9cd80f32f30ad7ff795ea2eb8
DEPS="python3 python3-flexcache python3-flexparser python3-platformdirs python3-typing-extensions"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-flexcache python3-flexparser python3-platformdirs python3-typing-extensions"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
