# can-isotp: ISO 15765-2 (ISO-TP) transport for Python. Its TransportLayer
# is the protocol in Python and runs on any python-can bus
# (isotp.CanStack / NotifierBasedCanStack: on QNX the socketcan interface on
# qnx-socketcan's buses). isotp.socket, the Linux kernel's CAN_ISOTP socket,
# is not available on QNX (qnx-socketcan has CAN_RAW only). Pure Python: the
# published wheel is installed as released (scripts/lib/pywheel.sh).
VERSION=2.0.7
TARBALL=https://files.pythonhosted.org/packages/e4/1e/f1b163783c90ccbad0227de267d899d7283b3174b89f1d66bb7c2a509808/can_isotp-2.0.7-py3-none-any.whl
SHA256=28d155dec1c640bf32771fb4e271323d777158ab130372724a8b7b82aca66a7c
DEPS="python3 python3-can"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-can"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
