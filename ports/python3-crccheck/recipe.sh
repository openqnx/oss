# crccheck: CRC and checksum algorithms (over 170 named CRCs) in pure Python;
# cantools computes AUTOSAR end-to-end and SecOC checksums with it. The
# published wheel is installed as released (scripts/lib/pywheel.sh).
VERSION=1.3.1
TARBALL=https://files.pythonhosted.org/packages/01/b5/68a9054be852e61f31de1be4e8d95802646b93344ce17c2380a1748706fa/crccheck-1.3.1-py3-none-any.whl
SHA256=1680c9a7bb1ca4bec45fa19b8ca64319f10d2ce4eb8b0d25d51cb99a20ca0108
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
