# pint: physical quantities (a value and its unit), unit conversion and
# dimensional analysis. 0.24.x is the series python3-obd pins (pint==0.24.*).
# Pure Python: the published wheel is installed as released
# (scripts/lib/pywheel.sh).
VERSION=0.24.4
TARBALL=https://files.pythonhosted.org/packages/b7/16/bd2f5904557265882108dc2e04f18abc05ab0c2b7082ae9430091daf1d5c/Pint-0.24.4-py3-none-any.whl
SHA256=aa54926c8772159fcf65f82cc0d34de6768c151b32ad1deb0331291c38fe7659
DEPS="python3 python3-flexcache python3-flexparser python3-platformdirs python3-typing-extensions"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-flexcache python3-flexparser python3-platformdirs python3-typing-extensions"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
