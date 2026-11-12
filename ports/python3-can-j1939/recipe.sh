# can-j1939: SAE J1939 for Python on python-can -- electronic control unit,
# controller applications, address claiming, PGN requests and the transport
# protocol (BAM and RTS/CTS) in Python (no kernel CAN_J1939 needed). Pure
# Python: the published wheel is installed as released (scripts/lib/pywheel.sh).
VERSION=2.0.12
TARBALL=https://files.pythonhosted.org/packages/13/16/6cc5a93439f7b7ddbadb4e624e9217a504b7ff58265878627d77dae788c4/can_j1939-2.0.12-py2.py3-none-any.whl
SHA256=7c5791e6587a3f1a53cbd53f7acf493fff3bb187846dcacf05b382ab2570104d
DEPS="python3 python3-can numpy"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-can numpy"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
