# canmatrix: reads, writes, converts and compares CAN database formats
# (DBC, SYM, DBF, JSON, CSV as they are; ARXML, KCD, FIBEX with python3-lxml;
# YAML with python3-pyyaml), with the commands canconvert and cancompare.
# Pure Python: the published wheel is installed as released (scripts/lib/pywheel.sh).
VERSION=1.3.3
TARBALL=https://files.pythonhosted.org/packages/6e/49/8e51b4632d93dd7b79fb1b4173f0118977c5da21e2c9d05dee11ae4d3ab6/canmatrix-1.3.3-py3-none-any.whl
SHA256=a67ba771f37445c9af481dd14c662cd65ee11b212cf19e37a795396d1d373011
DEPS="python3 python3-attrs python3-click"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-attrs python3-click"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts $SRCROOT/*.whl; }
