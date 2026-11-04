# scikit-build-core: the PEP 517 build backend for CMake projects.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers (qnx_py_scripts).
VERSION=1.1.2
TARBALL=https://files.pythonhosted.org/packages/cb/8e/f13e5d7dc75fc979464d5bcc1bc762b07133df3d09d667c34727dfd8b0e8/scikit_build_core-1.1.2-py3-none-any.whl
SHA256=7f4f959a467692db50bdd1ea7ce1507f2c7972e6023b753ee302d169bc4cec85
DEPS="python3 python3-packaging python3-pathspec"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-packaging python3-pathspec"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
