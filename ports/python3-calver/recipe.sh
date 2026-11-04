# calver: calendar versioning for setuptools.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=2025.10.20
TARBALL=https://files.pythonhosted.org/packages/08/cc/31996b2bd918698888b443da1dea201c338643224539c1be4b2d76c9353c/calver-2025.10.20-py3-none-any.whl
SHA256=d7d75224eed9d9263f4fb30008487615196d208d14752bfd93fea7af2c84f508
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
