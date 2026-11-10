# PyFilesystem2: one API for files in memory, on disk, in archives
# (fs.memoryfs, fs.osfs, fs.tarfs, fs.zipfs, ...).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=2.4.16
TARBALL=https://files.pythonhosted.org/packages/b9/5c/a3d95dc1ec6cdeb032d789b552ecc76effa3557ea9186e1566df6aac18df/fs-2.4.16-py2.py3-none-any.whl
SHA256=660064febbccda264ae0b6bace80a8d1be9e089e0a5eb2427b7d517f9a91545c
DEPS="python3 python3-appdirs python3-setuptools python3-six"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-appdirs python3-setuptools python3-six"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
