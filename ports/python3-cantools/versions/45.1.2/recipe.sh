# cantools: CAN database tools -- reads and writes DBC, KCD, SYM, ARXML and
# CDD files, encodes and decodes messages by their signals, and the
# `cantools` command (decode, monitor, dump, convert, generate C source,
# plot). With python3-can: tester and monitor on a bus (on QNX the socketcan
# interface on qnx-socketcan's buses). Pure Python: the published wheel is
# installed as released (scripts/lib/pywheel.sh).
VERSION=45.1.2
TARBALL=https://files.pythonhosted.org/packages/fc/2c/c5d032465c92adfa385eecfbab342d34fdd118f55acc1698bdc794d06610/cantools-45.1.2-py3-none-any.whl
SHA256=2522d53d21ca689168c5dfcb49894ae867b3f82b0c779b72602f0fd077c6076c
DEPS="python3 python3-bitstruct python3-can python3-textparser python3-argparse-addons python3-crccheck"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-bitstruct python3-can python3-textparser python3-argparse-addons python3-crccheck"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts $SRCROOT/*.whl; }
