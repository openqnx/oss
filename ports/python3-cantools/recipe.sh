# cantools: CAN database tools -- reads and writes DBC, KCD, SYM, ARXML and
# CDD files, encodes and decodes messages by their signals, and the
# `cantools` command (decode, monitor, dump, convert, generate C source,
# plot). With python3-can: tester and monitor on a bus (on QNX the socketcan
# interface on qnx-socketcan's buses). Pure Python: the published wheel is
# installed as released (scripts/lib/pywheel.sh).
VERSION=45.2.0
TARBALL=https://files.pythonhosted.org/packages/6e/7a/baac7b7698df65c3362315a3e92a525a9496e7a88ea0dc0617cb0622cdcf/cantools-45.2.0-py3-none-any.whl
SHA256=6696219fc778da1b7377484fb7469e9908ec714294f88c1966017b80e72690fc
DEPS="python3 python3-bitstruct python3-can python3-textparser python3-argparse-addons python3-crccheck"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-bitstruct python3-can python3-textparser python3-argparse-addons python3-crccheck"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts $SRCROOT/*.whl; }
