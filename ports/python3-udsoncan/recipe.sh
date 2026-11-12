# udsoncan: Unified Diagnostic Services (ISO 14229) client for Python --
# the services, their requests and responses, and connections over ISO-TP
# (PythonIsoTpConnection on python3-can-isotp), J2534 and others. Pure
# Python: the published wheel is installed as released (scripts/lib/pywheel.sh).
VERSION=1.26.1
TARBALL=https://files.pythonhosted.org/packages/9b/99/5c79ce34af274fe20c4511ecf792ed2d868ad64a2406351f20df7f2b33fd/udsoncan-1.26.1-py3-none-any.whl
SHA256=c1bd6cc54d6c1124c0eafeff2ffaab48bfeb7d0d2576959f21840c37e96dfb9a
DEPS="python3 python3-can-isotp"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-can-isotp"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
