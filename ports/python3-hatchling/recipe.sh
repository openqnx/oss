# hatchling: the build backend of Hatch.
# Version 1.31.0: the last release before tomlkit became a dependency.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers (qnx_py_scripts).
VERSION=1.31.0
TARBALL=https://files.pythonhosted.org/packages/64/e2/2c0af0a52d16be74a4f194564fcdc417521ed863e9b65e4bc9052dacba6f/hatchling-1.31.0-py3-none-any.whl
SHA256=aac80bec8b6fe35e8480f1c335be8910fa210a0e6f735a139be205dadcacb544
DEPS="python3 python3-packaging python3-pathspec python3-pluggy python3-trove-classifiers"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-packaging python3-pathspec python3-pluggy python3-trove-classifiers"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
