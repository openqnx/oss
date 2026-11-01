# colcon-core: command line tool to build sets of software packages.
# colcon-core's metadata asks for setuptools <80; this tree's python3-setuptools
# is 80.8.0. colcon reads it only through setup.py-based package
# identification (colcon-python-setup-py), which works with 80 as well; the
# pin is not checked at run time (colcon loads its extensions through
# importlib.metadata entry points).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.21.3
TARBALL=https://files.pythonhosted.org/packages/8f/ab/f52f1c8410518f4419ed7bb06550c9bb26acf1928848ae23136c0dbd51e5/colcon_core-0.21.3-py3-none-any.whl
SHA256=033b06d77f972ce043efcbd7058a8020892d671e5b862dbb2cebe866a03e8ae7
DEPS="python3 python3-distlib python3-empy python3-packaging python3-pytest python3-pytest-cov python3-pytest-repeat python3-pytest-rerunfailures python3-setuptools"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-distlib python3-empy python3-packaging python3-pytest python3-pytest-cov python3-pytest-repeat python3-pytest-rerunfailures python3-setuptools"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
# The wheel's console script (colcon = colcon_core.command:main) is written
# by pip at install time; unpacking the wheel does not, so it is made here.
do_install() {
  qnx_py_install_pure || return 1
  mkdir -p "$STAGE$PREFIX/bin" && printf '%s\n' "#!$PREFIX/bin/python3" "import sys" \
    "from colcon_core.command import main" "sys.exit(main())" > "$STAGE$PREFIX/bin/colcon" && \
  chmod 755 "$STAGE$PREFIX/bin/colcon"
}
