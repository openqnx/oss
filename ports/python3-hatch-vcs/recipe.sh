# hatch-vcs: version from version control for hatchling.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.5.0
TARBALL=https://files.pythonhosted.org/packages/5f/48/1f85cee4b7b4f40b9b814b1febbc661bda6ced9649e410a0b74f6e415dd0/hatch_vcs-0.5.0-py3-none-any.whl
SHA256=b49677dbdc597460cc22d01b27ab3696f5e16a21ecf2700fb01bc28e1f2a04a7
DEPS="python3 python3-hatchling python3-setuptools_scm"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-hatchling python3-setuptools_scm"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
